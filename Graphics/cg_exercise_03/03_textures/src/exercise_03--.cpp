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
	int s = std::floor(uv.x * mip_levels[level]->getWidth());
	int t = std::floor(uv.y * mip_levels[level]->getHeight());

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
		return 0;
	else if (val >= size)
		return size - 1;
	else return val;
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
	if (val < 0 || val >= size)
	{
		int temp = val % size;
		return (temp < 0) ? size + temp : temp;
	}
	else return val;
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
	const glm::vec2 st{ uv.x * width, uv.y * height };

	//texel coordinates
	// (x1, y1)    (x2, y1)
	//
	// (x1, y2)    (x2, y2)

	//calculate x coords of neighbouring texels, weight a
	float mod_s = std::modf(st.x, &intpart);
	mod_s += (mod_s < 0.f) ? 1.f : 0.f; //fractional part should be positive

	float a = mod_s; //weight a
	int x1 = std::floor(st.x);
	int x2 = std::ceil(st.x);
	if (x1 == x2) //mod_s == 0
	{
		--x1;
		a = 0.5f;
	}
	else if (mod_s < 0.5f)
	{
		--x1;
		--x2;
		a += 0.5f;
	}
	else if (mod_s == 0.5f)
	{
		--x1;
		--x2;
		a = 1.0f;
	}
	else //mod_s > 0.5f
		a -= 0.5f;

	//y coords of neighbouring texels, weight b
	float mod_t = std::modf(st.y, &intpart);
	mod_t += (mod_t < 0.f) ? 1.f : 0.f; //fractional part should be positive

	float b = mod_t; //weight b
	int y1 = std::floor(st.y);
	int y2 = std::ceil(st.y);
	if (y1 == y2) //mod_t == 0
	{
		--y1;
		b = 0.5f;
	}
	else if (mod_t < 0.5f)
	{
		--y1;
		--y2;
		b += 0.5f;
	}
	else if (mod_t == 0.5f)
	{
		--y1;
		--y2;
		b = 1.0f;
	}
	else //mod_t > 0.5f
		b -= 0.5f;

	//linear interpolation horizontally
	glm::vec4 t12 = (1.f - a) * get_texel(level, x1, y1) + a * get_texel(level, x2, y1);
	glm::vec4 t34 = (1.f - a) * get_texel(level, x1, y2) + a * get_texel(level, x2, y2);

	//linear interpolation vertically
	return (1.f - b) * t12 + b * t34;
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

	int level = 0, x = 0, y = 0; //x, y: coordinates applied to new level
	int &smaller = (size_x < size_y) ? x : y;
	int &bigger = (size_x < size_y) ? y : x;

	//loop until smaller > 0
	for (x = size_x >> 1, y = size_y >> 1; smaller > 0; x >>= 1, y >>= 1)
	{
		//create new level
		mip_levels.emplace_back(new Image(x, y));
		++level;

		//fill level
		for (int i = 0; i < x; ++i)
			for (int j = 0; j < y; ++j)
			{
				int _x = i << 1, _y = j << 1; //coordinates applied to lower level
				//calculate average value of pixels on lower level and set new pixel
				glm::vec4 texel = ((mip_levels[level - 1]->getPixel(_x, _y) + mip_levels[level - 1]->getPixel(_x + 1, _y)
					+ mip_levels[level - 1]->getPixel(_x, _y + 1) + mip_levels[level - 1]->getPixel(_x + 1, _y + 1)) * 0.25f);
				//mip_levels[level]->setPixel(i, j, pixel);
				set_texel(level, i, j, texel);
			}
	}

	//countinue loop until bigger > 0
	smaller = 1; //smaller edge has only width/heigth 1 now
	for (; bigger > 0; bigger >>= 1)
	{
		mip_levels.emplace_back(new Image(x, y));
		++level;
		for (int i = 0; i < bigger; ++i)
		{
			int _c = i << 1; //coordinate applied to lower level
			if (size_x < size_y) //_c = _y
			{
				//calculate average value of pixels on lower level and set new pixel
				glm::vec4 texel = ((mip_levels[level - 1]->getPixel(0, _c) + mip_levels[level - 1]->getPixel(0, _c + 1)) * 0.5f);
				//mip_levels[level]->setPixel(0, i, pixel);
				set_texel(level, 0, i, texel);
			}
			else //_c = _x
			{
				//calculate average value of pixels on lower level and set new pixel
				glm::vec4 texel = ((mip_levels[level - 1]->getPixel(_c, 0) + mip_levels[level - 1]->getPixel(_c + 1, 0)) * 0.5f);
				//mip_levels[level]->setPixel(i, 0, pixel);
				set_texel(level, i, 0, texel);
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
		float t = 0.f;
		if (intersect_plane(rays[i].origin, rays[i].direction, isect.position, isect.normal, &t))
		{
			intersection_positions[i] = rays[i].origin + t * rays[i].direction;
		}
	}

	// compute uv coordinates from intersection positions
	glm::vec2 intersection_uvs[4];
	get_intersection_uvs(intersection_positions, isect, intersection_uvs);

	// TODO: compute dudv = length of sides of AABB in uv space
	float maxus = std::max({ intersection_uvs[0].x, intersection_uvs[1].x, intersection_uvs[2].x, intersection_uvs[3].x });
	float maxvs = std::max({ intersection_uvs[0].y, intersection_uvs[1].y, intersection_uvs[2].y, intersection_uvs[3].y });
	float minus = std::min({ intersection_uvs[0].x, intersection_uvs[1].x, intersection_uvs[2].x, intersection_uvs[3].x });
	float minvs = std::min({ intersection_uvs[0].y, intersection_uvs[1].y, intersection_uvs[2].y, intersection_uvs[3].y });

	return glm::vec2(maxus - minus, maxvs - minvs);
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
	//uv --> st
	glm::vec2 dsdt{ dudv.x * mip_levels[0]->getWidth(), dudv.y * mip_levels[0]->getHeight() };
	float T = std::log2f(std::max({ dsdt.x, dsdt.y })); //which levels to use

	float intpart, weight;
	int level1 = 0, level2 = 0;
	weight = std::modf(T, &intpart);
	level1 = std::floor(T);
	level2 = std::ceil(T);

	/*if (level1 < 0 || level1 >= mip_levels.size() || level2 < 0 || level2 >= mip_levels.size())
		std::cout << "Hibakezelés..." << level1 << " " << level2 << std::endl;*/

	//error check for non existing levels, then only bilinear interpolation with one level
	if (level1 < 0)
		return evaluate_bilinear(0, uv);
	if (level2 > mip_levels.size() - 1)
		return evaluate_bilinear(mip_levels.size() - 1, uv);

	if (level1 == level2)
		return evaluate_bilinear(level1, uv);

	//bilinear interpolation on level1 and level2
	glm::vec4 vec1 = evaluate_bilinear(level1, uv);
	glm::vec4 vec2 = evaluate_bilinear(level2, uv);

	//linear interpolation of different levels
	return (1 - weight) * vec1 + weight * vec2;
}

// -----------------------------------------------------------------------------

/*
 * Transform the given direction d using the matrix transform.
 *
 * The output direction must be normalized, even if d is not.
 */
glm::vec3 transform_direction(glm::mat4 const& transform, glm::vec3 const& d)
{	
	return glm::normalize(glm::vec3(transform * glm::vec4(d, 0.f)));
}

/*
 * Transform the given position p using the matrix transform.
 */
glm::vec3 transform_position(glm::mat4 const& transform, glm::vec3 const& p)
{
	return glm::vec3(transform * glm::vec4(p, 1.f));
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

	glm::mat3 transform{ tangent, normal, bitangent };

	return glm::normalize(transform * d);
}

// -----------------------------------------------------------------------------
// CG_REVISION dfce8167a2ff99a11f4b6ce59777acd3a80ed38f
