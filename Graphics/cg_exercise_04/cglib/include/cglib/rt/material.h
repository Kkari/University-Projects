#pragma once

#include <cglib/rt/texture.h>
#include <cglib/rt/texture_mapping.h>

class Intersection;

class Material
{
public:
    Material() :
        k_a(new ConstTexture(glm::vec3(0.f))),
        k_d(new ConstTexture(glm::vec3(0.18f))),
        k_s(new ConstTexture(glm::vec3(0.f))),
        k_r(new ConstTexture(glm::vec3(0.f))),
        k_t(new ConstTexture(glm::vec3(0.f))),
        normal(new ConstTexture(glm::vec3(0.5f, 0.5f, 1.f))),
        eta(glm::vec3(1.f)),
        n(0.f) 
    { }

    std::shared_ptr<Texture> k_a;    // ambient reflectance
    std::shared_ptr<Texture> k_d;    // diffuse reflectance
    std::shared_ptr<Texture> k_s;    // specular reflectance
    std::shared_ptr<Texture> k_r;    // reflection
    std::shared_ptr<Texture> k_t;    // transmission
    std::shared_ptr<Texture> normal; // normal map
    glm::vec3 eta; // index of refraction used to compute transmission rays
    float n;       // phong exponent
};

class MaterialSample
{
public:
    void evaluate(Material const& material, Intersection const& isect);
    
    glm::vec3 k_a; // ambient reflectance
    glm::vec3 k_d; // diffuse reflectance
    glm::vec3 k_s; // specular reflectance
    glm::vec3 k_r; // reflection
    glm::vec3 k_t; // transmission
    glm::vec3 eta; // index of refraction used to compute transmission rays, ior = 1 is air (default)
    glm::vec3 normal; // the normal, as taken from the normal map.
    float n;       // phong exponent
};

