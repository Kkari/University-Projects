#pragma once

#include <cglib/rt/ray.h>
#include <cglib/rt/intersection.h>
#include <cglib/rt/intersection_tests.h>

class Intersectable
{
public:
    virtual bool intersect(Ray const& ray, Intersection* isect) const = 0;
};

class Sphere : public Intersectable
{
public:
    Sphere(glm::vec3 const& center_,
           float radius_) :
           center(center_),
           radius(radius_)
    {
    }

    bool intersect(Ray const& ray, Intersection* isect) const
    {
        float t;
        if (intersect_sphere(ray.origin, ray.direction, center, radius, &t))
        {
            isect->t = t;
            isect->position  = ray.origin + t * ray.direction;
            isect->normal    = glm::normalize(isect->position - center);
			isect->geometric_normal = isect->normal;
            return true;
        }
        return false;
    }

private:
    const glm::vec3 center;
    const float radius;
};

class Plane : public Intersectable
{
public:
    Plane(glm::vec3 const& center_,
          glm::vec3 const& normal_) :
          center(center_),
          normal(normal_)
    {
    }

    bool intersect(Ray const& ray, Intersection* isect) const
    {
        float t;
        if (intersect_plane(ray.origin, ray.direction, center, normal, &t))
        {
            isect->t = t;
            isect->position = ray.origin + t * ray.direction;
            isect->geometric_normal = normal;
            isect->normal = normal;
            return true;
        }
        return false;
    }

protected:
    const glm::vec3 center;
    const glm::vec3 normal;
};

class Quad : public Plane
{
public:
    Quad(glm::vec3 const& center_,
          glm::vec3 const& e0_,
          glm::vec3 const& e1_) :
          Plane(center_, glm::normalize(glm::cross(
                glm::normalize(e0_),
                glm::normalize(e1_)))),
          e0(e0_),
          e1(e1_),
          p(center_-0.5f*e0_-0.5f*e1_),
          len_e0_sq(glm::length(e0_)*glm::length(e0_)),
          len_e1_sq(glm::length(e1_)*glm::length(e1_))
    {
    }

    bool intersect(Ray const& ray, Intersection* isect) const
    {
        float t;
        if (intersect_plane(ray.origin, ray.direction, center, normal, &t))
        {
            isect->t = t;
            isect->position = ray.origin + t * ray.direction;
			isect->geometric_normal = normal;
            isect->normal = normal;

            const glm::vec3 d = isect->position - p;
            const float u = glm::dot(d, e0) / (len_e0_sq);
            if (u < 0.f || u > 1.f)
                return false;
            
            const float v = glm::dot(d, e1) / (len_e1_sq);
            if (v < 0.f || v > 1.f)
                return false;

            return true;
        }
        return false;
    }

private:
    const glm::vec3 e0;
    const glm::vec3 e1;
    const glm::vec3 p;
    const float len_e0_sq;
    const float len_e1_sq;
};
