#pragma once

#include <glm/glm.hpp>

#include <algorithm>

class Ray
{
public:
    Ray() :
        origin(0.f),
        direction(0.f)
    {
    }

    Ray(glm::vec3 const& origin,
        glm::vec3 const& direction) :
        origin(origin),
        direction(glm::normalize(direction))
    {
    }

    glm::vec3 origin;
    glm::vec3 direction;
};
