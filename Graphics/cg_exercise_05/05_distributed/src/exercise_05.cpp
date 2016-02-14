#include <cglib/rt/renderer.h>

#include <cglib/rt/epsilon.h>
#include <cglib/rt/intersection.h>
#include <cglib/rt/object.h>
#include <cglib/rt/light.h>
#include <cglib/rt/ray.h>
#include <cglib/rt/raytracing_context.h>
#include <cglib/rt/render_data.h>
#include <cglib/rt/scene.h>
#include <cglib/core/glmstream.h>
#include <exception>
#include <stdexcept>

#include <cglib/core/thread_local_data.h>

/*
 * Creates a random sample on a unit disk
 *
 * Parameters:
 * - u1: random number [0,1)
 * - u2: random number [0,1)
 */
glm::vec2
uniform_sample_disk(float u1, float u2)
{
	// TODO DOF: implement uniform sampling on a unit disk here
    float root = std::sqrt(u1);
    float omega = u2 * 2.f * M_PI;
    return glm::vec2(root * std::cos(omega), root * std::sin(omega));
}

/*
 * Creates a random sample on a unit sphere
 *
 * Parameters:
 * - u1: random number [0,1)
 * - u2: random number [0,1)
 */
glm::vec3
uniform_sample_sphere(float u1, float u2)
{
	// TODO AmbientOcclusion/IndirectIllumination: 
	// implement uniform sampling on a unit sphere
	float h = 1.f - (2.f * u1);
    float root = std::sqrt(1.f - std::pow(h,2));
    float omega = u2 * 2.f * M_PI;
    return glm::vec3(root * std::cos(omega), h, root * std::sin(omega));
}

/*
 * Creates a random sample on a hemisphere with
 * normal direction N
 *
 * Parameters:
 * - data: RenderData for access to random number generator
 * - N: main direction of the hemisphere
 */
glm::vec3
uniform_sample_hemisphere(RenderData & data, glm::vec3 const& N)
{
	// TODO AmbientOcclusion/IndirectIllumination: 
	// implement uniform sampling on a unit hemisphere.
	// data.tld->rand() creates uniform [0, 1] random numbers
	// TIP: use uniform_sample_sphere 
    glm::vec3 dir;
	do {
        dir = uniform_sample_sphere(data.tld->rand(), data.tld->rand());
    } while (glm::dot(N, dir) < FLT_EPSILON);

    return dir;
}

// returns a random unformly distributed point on the light source
glm::vec3 AreaLight::uniform_sample_point(float x0, float x1) const
{
	// TODO SoftShadow: Use the two random numbers to uniformly sample
	// a point on the area light
	
	return position + tangent * x0 + bitangent * x1;
}

glm::vec3 trace_recursive_with_lens(RenderData & data, Ray const& ray, int depth)
{
    glm::vec3 contrib(0.0f, 0.0f, 0.0f);

	if (data.context.params.dof && data.context.params.dof_rays > 0)
	{
		// TODO DOF: compute point on focus plane that is common to all rays of this pixel
        float focal_len = data.context.params.focal_length;
		glm::vec3 camDirection = data.context.scene->camera->get_direction();
        float t = focal_len / glm::dot(camDirection, ray.direction);
		
		glm::vec3 focus = glm::vec3(ray.origin + t * ray.direction);

		// TODO DOF: sample random points on the lens
		glm::mat4 ivm = data.context.scene->camera->get_inverse_view_matrix(data.camera_mode);

        for (int i = 0; i < data.context.params.dof_rays; i++)
        {
            glm::vec2 p = uniform_sample_disk(data.tld->rand(), data.tld->rand());
            p = p * data.context.params.lens_radius;

			// TODO DOF: generate ray from the sampled point on the
			// lens through the common point in the focus plane
            glm::vec4 p_world = ivm * glm::vec4(p.x, p.y, 0.f, 0.f);
            glm::vec3 pos = glm::vec3(p_world.x, p_world.y, p_world.z);

            pos += ray.origin;

            glm::vec3 dir = glm::normalize(focus - pos);
            Ray dofRay = Ray(pos, dir);
			// TODO DOF: start ray tracing with the new lens ray
            contrib += trace_recursive(data, dofRay, depth);
		}
		// TODO DOF: compute average contribution of all lens rays
        contrib = contrib / ((float)data.context.params.dof_rays);
	}
	else
        contrib = trace_recursive(data, ray, depth);

    return contrib;
}

float evaluate_ambient_occlusion(
	RenderData &data,           // class containing raytracing information
	glm::vec3 const& P,         // world space position
	glm::vec3 const& N)         // normal at the position (already normalized)
{
	// TODO AmbientOcclusion: compute ambient occlusion
	float ambient_occlusion = 0.f;

    for (int i = 0; i < data.context.params.ao_rays; ++i)
    {
        glm::vec3 p = uniform_sample_hemisphere(data, N);
        float cos_theta = glm::dot(glm::normalize(p), N);

        float dist = max_unobstructed_distance(data, P, glm::normalize(p));
		
		float V = 0.f;

        if (dist < FLT_MAX)
        {
			float c = 1.f / data.context.params.half_ao_radius / data.context.params.half_ao_radius;
            V = 1.f - 1.f / (1.f + c * std::pow(dist, 2));
		}
        else
        {
			V = 1.f;
		}
        ambient_occlusion += V * cos_theta;
	}
	ambient_occlusion *= 2.f / ((float)data.context.params.ao_rays);

	return ambient_occlusion;
}

glm::vec3 evaluate_illumination_from_light(
	RenderData &data,           // class containing raytracing information
	MaterialSample const& mat,  // the material at position
	Light const& light,         // the light source
	glm::vec3 const& LP,        // a point on the light source
	glm::vec3 const& P,         // world space position
	glm::vec3 const& N,         // normal at the position (already normalized)
	glm::vec3 const& V)         // view vector (already normalized)
{
    glm::vec3 dir_to_light = LP - P;                       // direction to the light
    const float squared_dtl = glm::dot(dir_to_light, dir_to_light);         // compute squared distance to light point
    dir_to_light /= sqrt(squared_dtl);                           // normalize direction

	float visibility = 1.f;
	if (data.context.params.shadows) {
		if (!visible(data, P, LP)) {
			visibility = 0.f;
		}
	}	

    auto incoming_light = visibility * light.getEmission(-dir_to_light) / squared_dtl;
    return evaluate_phong_BRDF(data, mat, dir_to_light, N, V) * incoming_light;
}

glm::vec3 evaluate_illumination(
	RenderData &data,           // class containing raytracing information
	MaterialSample const& mat,  // the material at position
	glm::vec3 const& P,         // world space position
	glm::vec3 const& N,         // normal at the position (already normalized)
	glm::vec3 const& V,         // view vector (already normalized)
	int depth)                  // the current recursion depth
{
	glm::vec3 direct_illumination(0.f);

	if (data.context.params.soft_shadow)
	{
		for (auto& light : data.context.scene->area_lights)
		{
			// TODO SoftShadow: sample point on light source for soft shadows
			glm::vec3 current_direct_illumination(0.f);

            for (int i = 0; i < data.context.params.shadow_rays; i++)
            {
                glm::vec3 light_point = light->uniform_sample_point(data.tld->rand(), data.tld->rand());

                if (visible(data, P, light_point))
                {
                    glm::vec3 p_coeff = evaluate_phong_BRDF(data, mat, glm::normalize(light_point - P), N, V); //f(izé) * cos(theta(i))
                    glm::vec3 dir_to_light = light->getEmission(glm::normalize(P - light_point));
                    float cos_thetha0 = 1.f;
                    current_direct_illumination += p_coeff * dir_to_light * cos_thetha0/ glm::dot(P - light_point, P - light_point);
					//current_direct_illumination += glm::vec3(1.f);
				}
			}

			current_direct_illumination *= light->get_area() / data.context.params.shadow_rays;
			direct_illumination += current_direct_illumination;
			(void)light; // prevent unused warning*/
		}
		//direct_illumination /= 100.f; //random leosztottam 100-al hogy lássam mi van a nagyon fehér részeken
	}
    else
    {
        for (auto& l : data.context.scene->lights)
        {
            const glm::vec3 light_position = l->getPosition();
			direct_illumination += evaluate_illumination_from_light(
                    data, mat, *l, light_position, P, N, V);
		}

		direct_illumination /= data.context.scene->lights.size();
	}

	glm::vec3 indirect_illumination(0.f);

	if (data.context.params.indirect) 
	{
		// TODO IndirectIllumination: compute indirect illumination
        for (int i = 0; i < data.context.params.indirect_rays; i++)
        {
            glm::vec3 ray_dir = uniform_sample_hemisphere(data, N);
            glm::vec3 trace_rec = trace_recursive(data, Ray(P, ray_dir), depth+1);
            glm::vec3 p_coeff = evaluate_phong_BRDF(data, mat, ray_dir, N, V);

            indirect_illumination += p_coeff * trace_rec;
		}

		indirect_illumination *= 2 * M_PI / (float)data.context.params.indirect_rays;
	}

	return direct_illumination + indirect_illumination;
}
// CG_REVISION 11b5702e34b37bad947c1f3cb241cd9fe1dc6bc4
