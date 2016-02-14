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
	int stack[64];
	int stack_size = 0;

	float min_dist = std::numeric_limits<float>::max();
	glm::vec3 bary(0.f);
	bool hit = false;
	int nearest_triangle = -1;
	
	glm::vec3 div = 1.0f / ray.direction;

	{ /* push root node on stack if hit */
		float t_min = 0.0;
		if(nodes[0].aabb.intersect(ray, t_min, min_dist, div))
			stack[stack_size++] = 0;
	}

	while(stack_size > 0) {
		const Node &n = nodes[stack[--stack_size]];
		if(n.left < 0) { /* leaf node, intersect triangles */
			for(int i = 0; i < n.num_triangles; i++) {
				int x = triangle_indices[n.triangle_idx + i];
				float dist;
				glm::vec3 b;
				if(intersect_triangle(ray.origin, ray.direction,
						triangle_soup.vertices[x * 3 + 0],
						triangle_soup.vertices[x * 3 + 1],
						triangle_soup.vertices[x * 3 + 2], 
						b, dist)) {
					hit = true;
					if(dist < min_dist || nearest_triangle == -1) {
						min_dist = dist;
						bary = b;
						cg_assert(x >= 0);
						nearest_triangle = x;
					}
				}
			}
		}
		else {
			float t_min_l = 0;
			float t_max_l = min_dist;
			float t_min_r = 0;
			float t_max_r = min_dist;

			bool il = nodes[n.left ].aabb.intersect(ray, t_min_l, t_max_l, div);
			bool ir = nodes[n.right].aabb.intersect(ray, t_min_r, t_max_r, div);
			if(!il && !ir) { /* no child hit, do nothing */
			}
			else if(il ^ ir) { /* only one child hit */
				stack[stack_size++] = il ? n.left : n.right;
			}
			else { /* both children hit, order by first aabb intersection */
				if(t_min_l < t_min_r) {
					stack[stack_size++] = n.right;
					stack[stack_size++] = n.left;
				}
				else {
					stack[stack_size++] = n.left;
					stack[stack_size++] = n.right;
				}
			}
		}
	}

	if (isect && hit) {
		triangle_soup.fill_intersection(isect, nearest_triangle, min_dist, bary);
	}
	return hit;
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
	unsigned const num_nodes = static_cast<unsigned>(nodes.size());
	cg_assert((triangle_soup.num_triangles == 0 && num_nodes == 1)
		   || num_nodes <= unsigned(2 * triangle_soup.num_triangles));

	int sum_triangles = 0;
	for(unsigned int i = 0; i < num_nodes; i++) {
		const Node &n = nodes[i];
		cg_assert((n.left != n.right)
				|| (n.left == -1 && n.right == -1));
		cg_assert((n.left  != -1) || (n.left == -1 && n.right == -1));
		cg_assert((n.right != -1) || (n.left == -1 && n.right == -1));
		cg_assert(n.left < int(num_nodes) && n.right < int(num_nodes));
		if(n.left == -1)
			sum_triangles += n.num_triangles;
	}
	cg_assert(sum_triangles == triangle_soup.num_triangles);
}

void BVH::
build_bvh(int node_idx, int first_triangle_idx, int num_triangles, int depth)
{	
	cg_assert(node_idx >= 0);
	cg_assert(node_idx < int(nodes.size()));
	Node& node = nodes[node_idx];

	if (node_idx == 0 && num_triangles == 0)
	{
		return;
	}
	cg_assert(num_triangles > 0);
	cg_assert(first_triangle_idx + num_triangles <= triangle_soup.num_triangles);

	int axis = depth % 3; /* split axis */
	if(num_triangles <= MAX_TRIANGLES_IN_LEAF) {
		node.left          = -1;
		node.right         = -1;
		node.triangle_idx  = first_triangle_idx;
		node.num_triangles = num_triangles;
		for(int i = 0; i < num_triangles; i++) {
			int tidx = triangle_indices[first_triangle_idx + i];
			for(int j = 0; j < 3; j++)
				node.aabb.extend(triangle_soup.vertices[tidx * 3 + j]);
		}
	}
	else {
		std::nth_element(
				triangle_indices.begin() + first_triangle_idx,
				triangle_indices.begin() + first_triangle_idx + num_triangles / 2,
				triangle_indices.begin() + first_triangle_idx + num_triangles,
				[&](int l, int r) -> bool {
					auto &v = triangle_soup.vertices;
					float min_l, min_r, max_l, max_r;
					min_l = min_r =  FLT_MAX;
					max_l = max_r = -FLT_MAX;
					for(int i = 0; i < 3; i++) {
						min_l = std::min(min_l, v[l * 3 + i][axis]);
						max_l = std::max(max_l, v[l * 3 + i][axis]);
						min_r = std::min(min_r, v[r * 3 + i][axis]);
						max_r = std::max(max_r, v[r * 3 + i][axis]);
					}
					return min_l + max_l < min_r + max_r;
				});
		int const num_nodes = static_cast<int>(nodes.size());
		node.left  = num_nodes + 0;
		node.right = num_nodes + 1;
		nodes.push_back(Node());
		nodes.push_back(Node());
		node.triangle_idx = first_triangle_idx;
		node.num_triangles = num_triangles;
		int nt = num_triangles / 2;
		build_bvh(node.left, first_triangle_idx, nt, depth + 1);
		build_bvh(node.right, first_triangle_idx + nt, num_triangles - nt, depth + 1);

		Node &nl = nodes[node.left];
		Node &nr = nodes[node.right];

		node.aabb.min = glm::min(nl.aabb.min, nr.aabb.min);
		node.aabb.max = glm::max(nl.aabb.max, nr.aabb.max);
	}
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
