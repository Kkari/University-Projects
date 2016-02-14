#include <cglib/rt/bvh.h>
#include <cglib/rt/light.h>
#include <cglib/rt/texture.h>
#include <cglib/rt/intersection.h>
#include <cglib/rt/triangle_soup.h>
#include <cglib/rt/interpolate.h>

#include <cglib/core/camera.h>

BVH::
BVH(const TriangleSoup &triangle_soup_)
	: triangle_soup(triangle_soup_)
	, triangle_indices(triangle_soup_.num_triangles)
	, nodes(1)
{
	nodes.reserve(triangle_soup.num_triangles * 2);
	for(int i = 0; i < triangle_soup.num_triangles; i++)
		triangle_indices[i] = i;
	build_bvh(0, 0, triangle_soup.num_triangles, 0);

	sanity_checks();
}

bool BVH::
intersect_local(Ray const& ray, Intersection* isect) const
{
	float t_min = 0.0f;
	float t_max = FLT_MAX;

	if(!nodes[0].aabb.intersect(ray, t_min, t_max))
		return false;

	return intersect_recursive(ray, 0, &t_max, isect);
}

bool BVH::
intersect(Ray const& ray, Intersection* isect) const
{
	// transform ray in object space
	const Ray ray_local = transform_ray(ray, transform_world_to_object);
	Intersection isect_local;
	if (intersect_local(ray_local, &isect_local)) {
		if (isect) {
			*isect = transform_intersection(isect_local, 
				transform_object_to_world, transform_object_to_world_normal);
			isect->t = glm::length(ray.origin-isect->position);
		}
		return true;
	}
	return false;
}

void BVH::
sanity_checks()
{
}

glm::vec3 BVH::
intersect_count(const Ray &ray, int idx, int depth)
{
	Ray ray_local = ray;
	if (depth == 0)
		ray_local = transform_ray(ray, transform_world_to_object);

	glm::vec3 colors[5] = {
		glm::vec3(1.0, 0.0, 0.0),
		glm::vec3(0.0, 1.0, 0.0),
		glm::vec3(0.0, 0.0, 1.0),
		glm::vec3(1.0, 0.0, 1.0),
		glm::vec3(0.0, 1.0, 1.0),
	};
	const Node &n = nodes[idx];
	float t_min = 0.0;
	float t_max = FLT_MAX;

	if(!n.aabb.intersect(ray_local, t_min, t_max))
		return glm::vec3(0.0f);

	if(n.left < 0) {
		return colors[depth % 5];
	}
	else {
		return colors[depth % 5]
			+ intersect_count(ray_local, n.left,  depth + 1)
			+ intersect_count(ray_local, n.right, depth + 1);
	}
}

void BVH::
compute_shading_info(Intersection* isect) {
	cg_assert(isect);
	auto &material_ = triangle_soup.materials[triangle_soup.material_ids[isect->primitive_id]];
	isect->material.evaluate(material_, *isect);
}

void BVH::
compute_shading_info(const Ray rays[4], Intersection* isect) {
	cg_assert(isect);
	glm::vec2 uv_min = glm::vec2( std::numeric_limits<float>::max());
	glm::vec2 uv_max = glm::vec2(-std::numeric_limits<float>::max());
	auto t_id = isect->primitive_id;
	cg_assert(t_id < unsigned(triangle_soup.num_triangles));
	for(int i = 0; i < 4; i++) {
		glm::vec3 b;
		float d;
		intersect_triangle<false>(rays[i].origin, rays[i].direction,
				triangle_soup.vertices[t_id * 3 + 0],
				triangle_soup.vertices[t_id * 3 + 1],
				triangle_soup.vertices[t_id * 3 + 2],
				b, d);

		glm::vec2 uv = interpolate_barycentric(
				triangle_soup.tex_coordinates[t_id * 3 + 0],
				triangle_soup.tex_coordinates[t_id * 3 + 1],
				triangle_soup.tex_coordinates[t_id * 3 + 2], b);

		uv_min = glm::min(uv_min, uv);
		uv_max = glm::max(uv_max, uv);
	}

	isect->dudv = glm::abs(uv_max - uv_min);
	auto &material_ = triangle_soup.materials[triangle_soup.material_ids[isect->primitive_id]];
	isect->material.evaluate(material_, *isect);
}
