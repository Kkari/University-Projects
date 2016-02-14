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
    glm::vec2 get_uv(Intersection const&) const override
    {
        return glm::vec2(0.f, 0.f);
    }
};

class UVMapping : public TextureMapping
{
public:
    glm::vec2 get_uv(Intersection const& isect) const override;
    void compute_tangent_space(Intersection* isect) const override final;
};

class SphericalMapping : public TextureMapping
{
    glm::vec3 center;
    glm::vec2 scale_uv;

public:
    SphericalMapping(
		glm::vec3 const& center_,
		glm::vec2 const& scale_uv_ = glm::vec2(1.f)) :
		center(center_),
		scale_uv(scale_uv_)
	{}

    glm::vec2 get_uv(Intersection const& isect) const override;
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
        glm::vec3 const& p_,
        glm::vec3 const& normal_,
        glm::vec3 const& tangent_,
        glm::vec3 const& bitangent_,
        glm::vec2 const& scale_uv_ = glm::vec2(1.f)) :
        p(p_),
        normal(normal_),
        tangent(tangent_),
        bitangent(bitangent_),
        scale_uv(scale_uv_)
    {}

    glm::vec2 get_uv(Intersection const& isect) const override;
    void compute_tangent_space(Intersection* isect) const override final;
};

