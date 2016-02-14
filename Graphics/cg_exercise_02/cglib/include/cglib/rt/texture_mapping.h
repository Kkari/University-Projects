#pragma once

#include <glm/glm.hpp>

class Intersection;

class TextureMapping
{
public:
    virtual glm::vec2 get_uv(Intersection const& isect) const = 0;
};

class ZeroMapping : public TextureMapping
{
public:
    glm::vec2 get_uv(Intersection const&) const
    {
        return glm::vec2(0.f, 0.f);
    }
};

