#pragma once

#include <glm/glm.hpp>
#include <cglib/core/assert.h>

template<bool enable_early_out = true>
inline bool
intersect_triangle(
        glm::vec3 const& ray_origin,
        glm::vec3 const& ray_direction,
		glm::vec3 const& v0, 
		glm::vec3 const& v1, 
		glm::vec3 const& v2, 
        glm::vec3 & bary,
		float &dist)
{
	const glm::vec3 edge1 = v1 - v0;
	const glm::vec3 edge2 = v2 - v0;

	const glm::vec3 pvec = glm::cross(ray_direction, edge2);

	const float det = glm::dot(edge1, pvec);
	const float inv_det = 1.0f / det;

	const glm::vec3 tvec = ray_origin - v0;

	const float alpha = glm::dot(tvec, pvec) * inv_det;

	if (enable_early_out && (!(0.0f <= alpha) || alpha > 1.0f))
		return false;

	const glm::vec3 qvec = glm::cross(tvec, edge1);

	const float beta = glm::dot(ray_direction, qvec) * inv_det;

	if (enable_early_out && (!(0.0f <= beta) || alpha + beta > 1.f))
		return false;

	/* distance */
	const float t = glm::dot(edge2, qvec) * inv_det;
    if (enable_early_out && t <= 0.f)
        return false;

    bary = glm::vec3(1.f-alpha-beta, alpha, beta);

	dist = t;

	if(enable_early_out) {
		return true;
	}
	else {
		return glm::all(glm::greaterThanEqual(glm::vec4(bary, t), glm::vec4(0.0f)));
	}
}

inline bool 
intersect_sphere(
    glm::vec3 const& ray_origin,    // starting point of the ray
    glm::vec3 const& ray_direction, // direction of the ray
    glm::vec3 const& center,        // position of the sphere
    float radius,                   // radius of the sphere
    float* t)                       // output parameter which contains distance to the hit point
{
    cg_assert(t);

    const glm::vec3 e_c = ray_origin - center;
    const float c = glm::dot(e_c, e_c) - radius * radius;
    const float b = glm::dot(ray_direction, e_c);
    const float a = glm::dot(ray_direction, ray_direction);

    const float d = b * b - a * c;
    if (d >= 0.0f)
    {
        const float e = sqrt(d);
        const float f = 1.0f / a;
        const float t1 = (-b + e) * f;
        const float t2 = (-b - e) * f;

        const bool t1valid = t1 >= 0.0f;
        const bool t2valid = t2 >= 0.0f;
        *t = (t1valid && t2valid ? glm::min(t1, t2)
            : (t1valid ? t1
            : (t2valid ? t2 
            : -1)));

        if (*t >= 0)
        {
            return true;
        }
    }
    return false;
}

inline bool 
intersect_plane(
    glm::vec3 const& ray_origin,
    glm::vec3 const& ray_direction,
    glm::vec3 const& center,
    glm::vec3 const& normal,
    float* t)
{
    cg_assert(t);
    const float denom = glm::dot(ray_direction, normal);
    if (denom == 0.f) {
        return false;
    }
    *t = glm::dot(center - ray_origin, normal) / denom;
    
    if (*t > 0.f) {
        return true;
    }
    return false;
}
