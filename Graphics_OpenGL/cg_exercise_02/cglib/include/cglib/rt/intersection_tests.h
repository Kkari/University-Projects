#pragma once

#include <glm/glm.hpp>
#include <cglib/core/assert.h>

bool 
intersect_sphere(
    glm::vec3 const& ray_origin,    // starting point of the ray
    glm::vec3 const& ray_direction, // direction of the ray
    glm::vec3 const& center,        // position of the sphere
    float radius,                   // radius of the sphere
    float* t);                      // output parameter which contains distance to the hit point

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
    
    if (*t > 1e-4f) {
        return true;
    }
    return false;
}
