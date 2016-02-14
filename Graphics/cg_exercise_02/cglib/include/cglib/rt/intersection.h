#pragma once

#include <glm/glm.hpp>

#include <memory>

#include <cglib/rt/material.h>

class Intersection
{
public:
    Intersection() :
        position(std::numeric_limits<float>::max()), 
        geometric_normal(std::numeric_limits<float>::max()), 
		normal(std::numeric_limits<float>::max()), 
        uv(0.f), 
        dudv(0.f),
        primitive_id(0),
        t(std::numeric_limits<float>::max())
    {}

    bool isValid() const 
    { 
        return t != std::numeric_limits<float>::max(); 
    }

    MaterialSample material;
    glm::vec3 position;
	glm::vec3 geometric_normal;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    glm::vec2 uv;                   // uv texture coordinates at the intersection point
    glm::vec2 dudv;                 // side lengths of the pixel footprint's AABB in uv space (for mipmap filter)
    uint32_t primitive_id;          // only used for triangle meshes
    float t;
};
