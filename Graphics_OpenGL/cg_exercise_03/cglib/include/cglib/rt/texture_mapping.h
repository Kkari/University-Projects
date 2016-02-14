#pragma once

#include <glm/glm.hpp>

class Intersection;

class TextureMapping
{
public:
    virtual glm::vec2 get_uv(Intersection const& isect) const = 0;
    virtual void compute_tangent_space(Intersection* isect) const;
};

class ZeroMapping : public TextureMapping
{
public:
    glm::vec2 get_uv(Intersection const&) const
    {
        return glm::vec2(0.f, 0.f);
    }
};

class UVMapping : public TextureMapping
{
public:
    glm::vec2 get_uv(Intersection const& isect) const;
    void compute_tangent_space(Intersection* isect) const override final;
};

class SphericalMapping : public TextureMapping
{
    glm::vec3 center;
    glm::vec2 scale_uv;

public:
    SphericalMapping(
		glm::vec3 const& center,
		glm::vec2 const& scale_uv = glm::vec2(1.f)) :
		center(center),
		scale_uv(scale_uv)
	{}

    glm::vec2 get_uv(Intersection const& isect) const;
};

class PlanarMapping : public TextureMapping
{
    glm::vec3 p;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    glm::vec2 scale_uv;

public:
    PlanarMapping(
        glm::vec3 const& p,
        glm::vec3 const& normal,
        glm::vec3 const& tangent,
        glm::vec3 const& bitangent,
        glm::vec2 const& scale_uv = glm::vec2(1.f)) :
        p(p),
        normal(normal),
        tangent(tangent),
        bitangent(bitangent),
        scale_uv(scale_uv)
    {}

    glm::vec2 get_uv(Intersection const& isect) const;
    void compute_tangent_space(Intersection* isect) const override final;
};

