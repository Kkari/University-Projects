#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <cglib/rt/raytracing_context.h>
#include <cglib/rt/intersection.h>
#include <cglib/rt/ray.h>

glm::vec3 transform_direction(glm::mat4 const& transform, glm::vec3 const& d);
glm::vec3 transform_position(glm::mat4 const& transform, glm::vec3 const& p);

glm::vec3 transform_direction_to_object_space(glm::vec3 const& d, glm::vec3 const& normal, glm::vec3 const& tangent, glm::vec3 const& bitangent);

inline Ray transform_ray(Ray const& ray, glm::mat4 const& transform)
{
	if (RaytracingContext::get_active()->params.transform_objects) {
		return Ray(transform_position(transform, ray.origin),
				   transform_direction(transform, ray.direction));
	}
	return ray;
}

inline Intersection transform_intersection(Intersection const& isect, glm::mat4 const& transform, glm::mat4 const& transform_normal)
{
	if (RaytracingContext::get_active()->params.transform_objects) {
		Intersection isect_t     = isect;
		isect_t.position         = transform_position(transform, isect.position);
		isect_t.geometric_normal = transform_direction(transform_normal, isect.geometric_normal);
		isect_t.normal           = transform_direction(transform_normal, isect.normal);
		isect_t.shading_normal   = transform_direction(transform_normal, isect.shading_normal);
		isect_t.tangent          = transform_direction(transform_normal, isect.tangent);
		isect_t.bitangent        = transform_direction(transform_normal, isect.bitangent);
		return isect_t;
	}
	return isect;
}

