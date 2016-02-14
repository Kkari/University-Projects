#include <cglib/rt/renderer.h>
#include <cglib/rt/intersection_tests.h>
#include <cglib/rt/raytracing_context.h>
#include <cglib/rt/intersection.h>
#include <cglib/rt/ray.h>
#include <cglib/rt/scene.h>
#include <cglib/rt/light.h>
#include <cglib/rt/material.h>
#include <cglib/rt/render_data.h>
#include <cmath>

/*
 * TODO: implement a ray-sphere intersection test here.
 * The sphere is defined by its center and radius.
 *
 * Return true, if (and only if) the ray intersects the sphere.
 * In this case, also fill the parameter t with the distance such that
 *    ray_origin + t * ray_direction
 * is the intersection point.
 */
bool intersect_sphere(
    glm::vec3 const& ray_origin,    // starting point of the ray
    glm::vec3 const& ray_direction, // direction of the ray
    glm::vec3 const& center,        // position of the sphere
    float radius,                   // radius of the sphere
    float* t)                       // output parameter which contains distance to the hit point
{
    cg_assert(t);
    cg_assert(std::fabs(glm::length(ray_direction) - 1.f) < EPSILON);
    
    glm::vec3 d = ray_direction, 
              c = center, 
              e = ray_origin;

    float a = glm::dot(d, d);
    float b = 2.f * glm::dot(d, (e - c));
    float c_d = float(glm::dot((e - c), (e - c)) - radius * radius);

    float d_d = b * b - 4.f * a * c_d;
    float t1, t2;

    if (!(d_d < 0)) 
    {

        t1 = (-1 * b + sqrtf(d_d)) / (2 * a);
        t2 = (-1 * b - sqrtf(d_d)) / (2 * a);

        if (t1 > 0 && t2 > 0) 
        {
            *t = fmin(t1, t2);
        } 
        else if (t2 > 0) 
        {
            *t = t2;
        } 
        else if (t1 > 0)
        {
            *t = t1;
        }
        else
        {
            return false;
        }

        return true;
    }
    return false;
}

/*
 * emission characteristic of a spotlight
 */
glm::vec3 SpotLight::getEmission(
		glm::vec3 const& omega // world space direction
		) const
{
	cg_assert(std::fabs(glm::length(omega) - 1.f) < EPSILON);
 
    float cos_theta = glm::dot(direction, omega) / (glm::length(direction) * glm::length(omega));
	return emission * (float) pow(fmax(0, cos_theta), falloff);
}

glm::vec3 evaluate_phong(
	RenderData &data,			// class containing raytracing information
	MaterialSample const& mat,	// the material at position
	glm::vec3 const& P,			// world space position
	glm::vec3 const& N,			// normal at the position (already normalized)
	glm::vec3 const& V)			// view vector (already normalized)
{
	cg_assert(std::fabs(glm::length(N) - 1.f) < EPSILON);
	cg_assert(std::fabs(glm::length(V) - 1.f) < EPSILON);

	glm::vec3 contribution(0.f);

	// iterate over lights and sum up their contribution
	for (auto& light_uptr : data.context.scene->lights) {
		// TODO: calculate the (normalized) direction to the light
		const Light *light = light_uptr.get();
		//glm::vec3 L(0.0f, 1.0f, 0.0f);
        glm::vec3 light_pos = light->getPosition();
        glm::vec3 L = glm::normalize(light_pos - P);
        
        //printf("%f %f %f pos\n", light_pos.x, light_pos.y, light_pos.z);
        //printf("%f %f %f norm\n", L.x, L.y, L.z);
        
        glm::vec3 E = light->getEmission(-L);
        
        float cos_theta = glm::dot(L, N);
        float cos_psi = glm::dot(glm::reflect(-L, N), V);

        float squared_distance = glm::distance(P, light_pos);
        squared_distance *= squared_distance; // to get the squared distance

		float visibility = 1.f;
        if (data.context.params.shadows) 
        {
            // TODO: check if light source is visible
            visibility = visible(data, light_pos, P) ? 1.f : 0.f;
		}

		glm::vec3 diffuse(0.f);
		if (data.context.params.diffuse) 
        {
			// TODO: compute diffuse component of phong model
            diffuse = mat.k_d * (float) fmax(0, cos_theta);
		}

		glm::vec3 specular(0.f);
		if (data.context.params.specular) 
        {
            if(cos_theta > 0) 
            {
                specular = mat.k_s * (float) pow(fmax(0.f, cos_psi), mat.n);
            }
		}

		glm::vec3 ambient = data.context.params.ambient ? mat.k_a : glm::vec3(0.0f);

		// TODO: modify this and implement the phong model as specified on the exercise sheet
		contribution +=  (visibility * (diffuse + specular) + ambient) / squared_distance * E;
	}

	return contribution;
}

glm::vec3 evaluate_reflection(
	RenderData &data,			// class containing raytracing information
	int depth,					// the current recursion depth
	glm::vec3 const& P,			// world space position
	glm::vec3 const& N,			// normal at the position (already normalized)
	glm::vec3 const& V)			// view vector (already normalized)
{
	// TODO: calculate reflective contribution by constructing and shooting a reflection ray.
	glm::vec3 R = reflect(V, N);
	return trace_recursive(data, Ray(P + R * data.context.params.ray_epsilon, R), ++depth);
}

glm::vec3 evaluate_transmission(
	RenderData &data,			// class containing raytracing information
	int depth,					// the current recursion depth
	glm::vec3 const& P,			// world space position
	glm::vec3 const& N,			// normal at the position (already normalized)
	glm::vec3 const& V,			// view vector (already normalized)
	float eta)					// the relative refraction index
{
	// TODO: calculate transmissive contribution by constructing and shooting a transmission ray.
	glm::vec3 contribution(0.f);

    if(refract(V, N, eta, &contribution))
    {
        return trace_recursive(data, Ray(P + contribution * data.context.params.ray_epsilon, contribution), ++depth);
    }
    
    return contribution;
}

glm::vec3 handle_transmissive_material_single_ior(
	RenderData &data,			// class containing raytracing information
	int depth,					// the current recursion depth
	glm::vec3 const& P,			// world space position
	glm::vec3 const& N,			// normal at the position (already normalized)
	glm::vec3 const& V,			// view vector (already normalized)
	float eta)					// the relative refraction index
{
	if (data.context.params.fresnel) 
    {
		// TODO: implement fresnel handling here.

        float f = fresnel(V, N, eta);
        glm::vec3 tr = evaluate_transmission(data, depth, P, N, V, eta);
        glm::vec3 re = evaluate_reflection(data, depth, P, N, V);
	    
        if(f >= 0) {
            return f * re + (1-f) * tr;
        }

        return glm::vec3(0.f);
    }
	else 
    {
		// just regular transmission
		return evaluate_transmission(data, depth, P, N, V, eta);
	}
}

glm::vec3 handle_transmissive_material(
	RenderData &data,					// class containing raytracing information
	int depth,							// the current recursion depth
	glm::vec3 const& P,					// world space position
	glm::vec3 const& N,					// normal at the position (already normalized)
	glm::vec3 const& V,					// view vector (already normalized)
	glm::vec3 const& eta_of_channel)	// relative refraction index of red, green and blue color channel
{
	if (data.context.params.dispersion && !(eta_of_channel[0] == eta_of_channel[1] && eta_of_channel[0] == eta_of_channel[2])) {
		// TODO: split ray into 3 rays (one for each color channel) and implement dispersion here
        
        glm::vec3 R = handle_transmissive_material_single_ior(data, depth, P, N, V, eta_of_channel[0]);
        glm::vec3 G = handle_transmissive_material_single_ior(data, depth, P, N, V, eta_of_channel[1]);
        glm::vec3 B = handle_transmissive_material_single_ior(data, depth, P, N, V, eta_of_channel[2]);

		return glm::vec3(R[0], G[1], B[2]);
	}
	else {
		// dont handle transmission, take average refraction index instead.
		const float eta = 1.f/3.f*(eta_of_channel[0]+eta_of_channel[1]+eta_of_channel[2]);
		return handle_transmissive_material_single_ior(data, depth, P, N, V, eta);
	}
	return glm::vec3(0.f);
}
// CG_REVISION b27b5056a58a9221eb3999b4ae8f60f5ab4868a3
