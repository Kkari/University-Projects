#pragma once

#include <cglib/rt/material.h>
#include <cglib/rt/texture.h>
#include <cglib/rt/ray.h>
#include <cglib/rt/intersectable.h>
#include <cglib/rt/intersection.h>

class Object
{
public:
    Object();
    virtual ~Object() {}

    virtual bool intersect(Ray const& ray, Intersection* isect) const;

    virtual void compute_shading_info(Intersection* isect);

    virtual void compute_shading_info(const Ray rays[4], Intersection* isect);

	void get_intersection_uvs(glm::vec3 const positions[4], Intersection const& isect, glm::vec2 uvs[4]);

	// compute texel footprint in uv-space
	glm::vec2 compute_uv_aabb_size(const Ray rays[4], Intersection const& isect);

    virtual glm::vec2 get_uv(Intersection const& isect);

	void set_transform_object_to_world(glm::mat4 const& T);

    std::shared_ptr<Intersectable> geo;
    std::shared_ptr<Material> material;
    std::shared_ptr<TextureMapping> texture_mapping;
	glm::mat4 transform_object_to_world;
	glm::mat4 transform_world_to_object;
	glm::mat4 transform_object_to_world_normal;
	glm::mat4 transform_world_to_object_normal;
};

std::unique_ptr<Object> create_sphere(
		glm::vec3 const& center,
		float radius,
		glm::vec2 const& scale_uv = glm::vec2(1.f));

std::unique_ptr<Object> create_plane(
		glm::vec3 const& center,
		glm::vec3 const& normal,
		glm::vec3 const& tangent,
		glm::vec3 const& bitangent,
		glm::vec2 const& scale_uv = glm::vec2(1.f));

std::unique_ptr<Object> create_plane(
		glm::vec3 const& center,
		glm::vec3 const& normal,
		glm::vec3 const& tangent,
		glm::vec2 const& scale_uv);

std::unique_ptr<Object> create_plane(
		glm::vec3 const& center,
		glm::vec3 const& normal);


std::unique_ptr<Object> create_quad(
		glm::vec3 const& center,
		glm::vec3 const& normal,
		glm::vec3 const& e0,
		glm::vec3 const& e1,
		glm::vec2 const& scale_uv = glm::vec2(1.f));
