#include <cglib/rt/texture.h>
#include <cglib/rt/texture_mapping.h>
#include <cglib/rt/intersection.h>
#include <cglib/rt/render_data.h>
#include <cglib/rt/ray.h>
#include <cglib/rt/object.h>
#include <cglib/rt/renderer.h>
#include <cglib/core/thread_local_data.h>

#include <cglib/core/image.h>
#include <cglib/core/glmstream.h>
#include <cglib/core/assert.h>

#include <algorithm>
#include <cmath>

// -----------------------------------------------------------------------------

/*
 * Evaluates a texture for the given uv-coordinate without filtering.
 *
 * This method transformes the uv-coordinate into a st-coordinate and
 * rounds down to integer pixel values.
 *
 * The parameter level in [0, mip_levels.size()-1] is the miplevel of
 * the texture that is to be evaluated.
 */
glm::vec4 ImageTexture::
evaluate_nearest(int level, glm::vec2 const& uv) const
{
	cg_assert(level >= 0 && level < static_cast<int>(mip_levels.size()));
	cg_assert(mip_levels[level]);

	// TODO: compute the st-coordinates for the given uv-coordinates and mipmap level
	int s = floor(mip_levels[level]->getWidth() * uv[0]);
	int t = floor(mip_levels[level]->getHeight() * uv[1]);

	// get the value of pixel (s, t) of miplevel level
	return get_texel(level, s, t);
}

// -----------------------------------------------------------------------------

/*
 * Implement clamping here.
 *
 * The input "val" may be arbitrary, including negative and very large positive values.
 * The method shall always return a value in [0, size).
 * Out-of-bounds inputs must be clamped to the nearest boundary.
 */
int ImageTexture::
wrap_clamp(int val, int size)
{
	cg_assert(size > 0);
	
    if (val < 0)
    {
        return 0;
    }
    else if (val >= size)
    {
        return size - 1;
    }

    return val;
}

/*
 * Implement repeating here.
 *
 * The input "val" may be arbitrary, including negative and very large positive values.
 * The method shall always return a value in [0, size).
 * Out-of-bounds inputs must be mapped back into [0, size) so that 
 * the texture repeats infinitely.
 */
int ImageTexture::
wrap_repeat(int val, int size)
{
	cg_assert(size > 0);

    int temp = val % size;
	return (temp < 0) ? temp + size : temp;
}

// -----------------------------------------------------------------------------


/*
 * Implement bilinear filtering here.
 *
 * Use mip_levels[level] as the image to filter.
 * The input uv coordinates may be arbitrary, including values outside [0, 1).
 *
 * Callers of this method must guarantee that level is valid, and
 * that mip_levels[level] is properly initialized. Compare the assertions.
 *
 * The color of a texel is to be interpreted as the color at the texel center.
 */

glm::vec4 ImageTexture::
evaluate_bilinear(int level, glm::vec2 const& uv) const
{
	cg_assert(level >= 0 && level < static_cast<int>(mip_levels.size()));
	cg_assert(mip_levels[level]);

	const int width = mip_levels[level]->getWidth(), height = mip_levels[level]->getHeight();

	float intpart = 0.f;
	const glm::vec2 st{ uv[0] * width, uv[1] * height };

	//texel coordinates
	// (x1, y1)    (x2, y1)
	//
	// (x1, y2)    (x2, y2)



	//y coords of neighbouring texels, weight b
	float mod_t = std::modf(st[1], &intpart);
	mod_t += (mod_t < 0.f) ? 1.f : 0.f; //fractional part should be positive

	float w2 = mod_t; //weight b
	int y1 = floor(st[1]);
	int y2 = ceil(st[1]);

	if (y1 == y2) //mod_t == 0
	{
		--y1;
		w2 = 0.5f;
	}
	else if (mod_t < 0.5f)
	{
		--y1;
		--y2;
		w2 += 0.5f;
	}
    else if (mod_t == 0.5f)
    {
        --y1;
        --y2;
        w2 = 1.0f;
    }
	else //mod_t > 0.5f
		w2 -= 0.5f;
	
    //calculate x coords of neighbouring texels, weight a
	float mod_s = std::modf(st[0], &intpart);
	mod_s += (mod_s < 0.f) ? 1.f : 0.f; //fractional part should be positive

	float w1 = mod_s; //weight 1
	int x1 = floor(st[0]);
	int x2 = ceil(st[0]);
	
    if (x1 == x2) //mod_s == 0
	{
		--x1;
		w1 = 0.5f;
	}
	else if (mod_s < 0.5f)
	{
		--x1;
		--x2;
		w1 += 0.5f;
	}
    else if (mod_s == 0.5f)
    {
		--x1;
		--x2;
		w1 = 1.0f;
    }
	else //mod_s > 0.5f
		w1 -= 0.5f;


	// linear interpolations
	glm::vec4 t34 = (1.f - w1) * get_texel(level, x1, y2) + w1 * get_texel(level, x2, y2);
    glm::vec4 t12 = (1.f - w1) * get_texel(level, x1, y1) + w1 * get_texel(level, x2, y1);

	//linear interpolation vertically
	return (1.f - w2) * t12 + w2 * t34;
}


// -----------------------------------------------------------------------------

/*
 * This method creates a mipmap hierarchy for
 * the texture.
 *
 * This is done by iteratively reducing the
 * dimenison of a mipmap level and averaging
 * pixel values until the size of a mipmap
 * level is [1, 1].
 *
 * The original data of the texture is stored
 * in mip_levels[0].
 *
 * You can allocale memory for a new mipmap level 
 * with dimensions (size_x, size_y) using
 *		mip_levels.emplace_back(new Image(size_x, size_y));
 */
void ImageTexture::
create_mipmap()
{
	/* this are the dimensions of the original texture/image */
	int size_x = mip_levels[0]->getWidth();
	int size_y = mip_levels[0]->getHeight();

	cg_assert("must be power of two" && !(size_x & (size_x - 1)));
	cg_assert("must be power of two" && !(size_y & (size_y - 1)));

    //x, y: coordinates applied to new level
	int level = 0, x = 0, y = 0;     
    // smaller
	int &s = (size_x < size_y) ? x : y; 
    // bigger
	int &b = (size_x < size_y) ? y : x; 

	//loop until smaller > 0
	for (x = size_x / 2, y = size_y / 2; s > 0; x /= 2, y /= 2)
	{
		//create new level
		mip_levels.emplace_back(new Image(x, y));
		++level;

		//fill level
		for (int i = 0; i < x; ++i)
			for (int j = 0; j < y; ++j)
			{
                //coordinates applied to lower level
				int _x = i * 2, _y = j * 2; 
				
                //calculate average value of pixels on lower level and set new pixel
				glm::vec4 texel = ((mip_levels[level - 1]->getPixel(_x, _y) + mip_levels[level - 1]->getPixel(_x + 1, _y)
					+ mip_levels[level - 1]->getPixel(_x, _y + 1) + mip_levels[level - 1]->getPixel(_x + 1, _y + 1)) * 0.25f);
				
                //mip_levels[level]->setPixel(i, j, pixel);
				set_texel(level, i, j, texel);
			}
	}

	//countinue loop until bigger > 0
	
    //smaller edge has only width/heigth 1 now
    s = 1; 	
    
    for (; b > 0; b /= 2)
	{
		mip_levels.emplace_back(new Image(x, y));
		++level;
		for (int i = 0; i < b; ++i)
		{
            //coordinate applied to lower level
			int _c = i * 2; 			
            
            if (size_x >= size_y) 
			{
				//calculate average value of pixels on lower level and set new pixel
				glm::vec4 texel = ((mip_levels[level - 1]->getPixel(_c, 0) + mip_levels[level - 1]->getPixel(_c + 1, 0)) / 2.f);
                
                // help function: mip_levels[level]->setPixel(0, i, pixel);
				set_texel(level, i, 0, texel);
			}
            else //_c = _y
			{
				//calculate average value of pixels on lower level and set new pixel
				glm::vec4 texel = ((mip_levels[level - 1]->getPixel(0, _c) + mip_levels[level - 1]->getPixel(0, _c + 1)) / 2.f);
				set_texel(level, 0, i, texel);
			}
		}
	}
}
/*
 * Compute the dimensions of the pixel footprint's AABB in uv-space.
 *
 * First intersect the four rays through the pixel corners with
 * the tangent plane at the given intersection.
 *
 * Then the given code computes uv-coordinates for these
 * intersection points.
 *
 * Finally use the uv-coordinates and compute the AABB in
 * uv-space. 
 *
 * Return width (du) and height (dv) of the AABB.
 *
 */
glm::vec2 Object::
compute_uv_aabb_size(const Ray rays[4], Intersection const& isect)
{
	// TODO: compute intersection positions
	glm::vec3 intersection_positions[4] = {
		isect.position, isect.position, isect.position, isect.position
	};

	for (int i = 0; i < 4; ++i) {
		// todo: compute intersection positions using a ray->plane
		// intersection

	float t;

	if (intersect_plane(rays[i].origin, rays[i].direction, isect.position, isect.normal, &t))
	{
	    intersection_positions[i] = rays[i].origin + rays[i].direction * t;
	}
	}

	// compute uv coordinates from intersection positions
	glm::vec2 intersection_uvs[4];
	get_intersection_uvs(intersection_positions, isect, intersection_uvs);

    glm::vec2 iu[4] = intersection_uvs;

    // Get the extremities on each axis.
    // Because its a 4 vertex polygon in row major order there are always 2
    // vertices that can be the searched value.
    
    float uvs_min_y = iu[0][1];
    float uvs_min_x = iu[0][0];
    float uvs_max_y = iu[0][1];
    float uvs_max_x = iu[0][0];
    
    for (int i = 0; i < 4; i++) 
    {
        uvs_min_y = fmin(iu[i][1], uvs_min_y);
        uvs_min_x = fmin(iu[i][0], uvs_min_x);
    
        uvs_max_y = fmax(iu[i][1], uvs_max_y);
        uvs_max_x = fmax(iu[i][0], uvs_max_x);
    }

	// TODO: compute dudv = length of sides of AABB in uv space
	return glm::vec2(uvs_max_x - uvs_min_x, uvs_max_y - uvs_min_y);
}

/*
 * Implement trilinear filtering at a given uv-position.
 *
 * Transform the AABB dimensions dudv in st-space and
 * take the maximal coordinate as the 1D footprint size T.
 *
 * Determine mipmap levels i and i+1 such that
 *		texel_size(i) <= T <= texel_size(i+1)
 *
 *	Hint: use std::log2(T) for that.
 *
 *	Perform bilinear filtering in both mipmap levels and
 *	linearly interpolate the resulting values.
 *
 */
glm::vec4 ImageTexture::
evaluate_trilinear(glm::vec2 const& uv, glm::vec2 const& dudv) const
{
	glm::vec2 dsdt{ dudv.x * mip_levels[0]->getWidth(), dudv.y * mip_levels[0]->getHeight() };
	float T = std::log2f(std::max({ dsdt.x, dsdt.y }));

	double intpart, weight;
    int level1 = 0, level2 = 0;
	weight = modf(T, &intpart);
	level1 = floor(T);
	level2 = ceil(T);

	if (level1 < 0)
		return evaluate_bilinear(0, uv);

	if (level2 > static_cast<long>(mip_levels.size() - 1))
		return evaluate_bilinear(mip_levels.size() - 1, uv);

	glm::vec4 vec1 = evaluate_bilinear(level1, uv);
	glm::vec4 vec2 = evaluate_bilinear(level2, uv);

	return (1 - static_cast<float>(weight)) * vec1 + static_cast<float>(weight) * vec2; 
	return glm::vec4(0.f);
}


// -----------------------------------------------------------------------------

/*
 * Transform the given direction d using the matrix transform.
 *
 * The output direction must be normalized, even if d is not.
 */
glm::vec3 transform_direction(glm::mat4 const& transform, glm::vec3 const& d)
{
    glm::vec4 dir(d.x, d.y, d.z, 0);
    glm::vec4 res(transform * dir);

    float norm = sqrt(res.x * res.x + res.y * res.y + res.z * res.z);
	return glm::vec3(res.x, res.y, res.z) / norm;
}

/*
 * Transform the given position p using the matrix transform.
 */
glm::vec3 transform_position(glm::mat4 const& transform, glm::vec3 const& p)
{
    glm::vec4 pos(p.x, p.y, p.z, 1);
    glm::vec4 res(transform * pos);
	return glm::vec3(res.x, res.y, res.z);
}

/*
 * Intersect with the ray, but do so in object space.
 *
 * First, transform ray into object space. Use the methods you have
 * implemented for this.
 * Then, intersect the object with the transformed ray.
 * Finally, make sure you transform the intersection back into world space.
 *
 * isect is guaranteed to be a valid pointer.
 * The method shall return true if an intersection was found and false otherwise.
 *
 * isect must be filled properly if an intersection was found.
 */
bool Object::
intersect(Ray const& ray, Intersection* isect) const
{
	cg_assert(isect);

	if (RaytracingContext::get_active()->params.transform_objects) {
		// TODO: transform ray, intersect object, transform intersection
		// information back
		Ray transformedRay = transform_ray(ray, Object::transform_world_to_object);
		if (geo->intersect(transformedRay, isect))
		{
			*isect = transform_intersection(*isect, Object::transform_object_to_world, Object::transform_object_to_world_normal);
			isect->t = glm::distance(ray.origin, isect->position);
			return true;
		}
		else return false;
	}
	return geo->intersect(ray, isect);
}




/*
 * Transform a direction from tangent space to object space.
 *
 * Tangent space is a right-handed coordinate system where
 * the tangent is your thumb, the normal is the index finger, and
 * the bitangent is the middle finger.
 *
 * normal, tangent, and bitangent are given in object space.
 * Build a matrix that rotates d from tangent space into object space.
 * Then, transform d with this matrix to obtain the result.
 * 
 * You may assume that normal, tangent, and bitangent are normalized
 * to length 1.
 *
 * The output vector must be normalized to length 1, even if d is not.
 */
glm::vec3 transform_direction_to_object_space(
	glm::vec3 const& d, 
	glm::vec3 const& normal, 
	glm::vec3 const& tangent, 
	glm::vec3 const& bitangent)
{
	cg_assert(std::fabs(glm::length(normal)    - 1.0f) < 1e-4f);
	cg_assert(std::fabs(glm::length(tangent)   - 1.0f) < 1e-4f);
	cg_assert(std::fabs(glm::length(bitangent) - 1.0f) < 1e-4f);
	
    glm::mat3 M(tangent, normal, bitangent);
    
    return glm::normalize(M * d);
}

// -----------------------------------------------------------------------------
// CG_REVISION dfce8167a2ff99a11f4b6ce59777acd3a80ed38f
