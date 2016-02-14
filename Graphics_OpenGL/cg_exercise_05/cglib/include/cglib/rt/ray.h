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

    Ray(glm::vec3 const& origin_,
        glm::vec3 const& direction_) :
        origin(origin_),
        direction(glm::normalize(direction_))
    {
    }

    glm::vec3 origin;
    glm::vec3 direction;
};
