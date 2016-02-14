#pragma once

#include <cglib/rt/aabb.h>
#include <cglib/rt/object.h>

#include <vector>
#include <string>
#include <algorithm>

class Intersection;
class TriangleSoup;

class BVH : public Object
{
public:
	/*
	 * The maximum number of triangles allowed in one leaf node.
	 * Never use "magic numbers" in your code. If you need to know how many
	 * triangles are allowed in leaf node, use this enum.
	 */
	enum { MAX_TRIANGLES_IN_LEAF = 4 };

	/*
	 * A BVH node.
	 *
	 * At any time, the following must hold:
	 * - left, right    must either both be -1, or must both point to valid child node indices.
	 * - triangle_idx   must always point to a valid index into triangle_soup.
	 * - num_triangles  must always be greater than 0.
	 */
	struct Node {
		AABB aabb;
		int left          = -1;
		int right         = -1;
		int triangle_idx  = -1;
		int num_triangles = 0;
	};

	/*
	 * The triangle soup for which this BVH is built.
	 */
	const TriangleSoup &triangle_soup;

	/*
	 * Indices into triangle_soup. Will be reordered during the build phase.
	 */
	std::vector<int> triangle_indices;

	/*
	 * The nodes contained in this BVH.
	 */
	std::vector<Node> nodes;

	/* 
	 * Construct (and build) a new BVH for the given triangle soup.
	 */
	BVH(const TriangleSoup &triangle_soup_);
    
	/*
	 * Intersect the given ray with this bvh.
	 */
    bool intersect(Ray const& ray, Intersection* isect) const override;
    
	/*
	 * For the given intersection, compute additional information needed
	 * for shading.
	 */
    virtual void compute_shading_info(Intersection* isect) override;
    virtual void compute_shading_info(const Ray rays[4], Intersection* isect) override;

	/*
	 * Sanity checks for the BVH structure. Currently unused, but feel
	 * free to implement your own checks in this method during testing.
	 */
	void sanity_checks();

	void build_bvh(int node_idx, int first_triangle_idx, int num_triangles, int depth);
	int reorder_triangles_median(int first_triangle_idx, int num_triangles, int axis);
	bool intersect_recursive(const Ray &ray, int idx, float *t_max, Intersection* isect) const;

	/*
	 * Used for debug visualization. Maps the number of AABBs that can be
	 * encountered along the given ray to a color.
	 */
	glm::vec3 intersect_count(const Ray &ray, int idx, int depth);

private:
	bool intersect_local(Ray const& ray, Intersection* isect) const;
};

