#pragma once

#include <cglib/rt/texture.h>

#include <glm/glm.hpp>

#include <iostream>
#include <vector>
#include <memory>

class Material;
class Intersection;
class ImageTexture;

class TriangleSoup
{
public:
	std::vector<glm::vec3> vertices, normals;
	std::vector<glm::vec2> tex_coordinates;
    std::vector<int> material_ids;
    std::vector<Material> materials;
	int num_triangles = 0;

	TriangleSoup();

	TriangleSoup(std::vector<glm::vec3>&& vertices,
				 std::vector<glm::vec3>&& normals,
				 std::vector<glm::vec2>&& tex_coordinates,
				 std::vector<int>&&       material_ids,
				 std::vector<Material>&&  materials);

	TriangleSoup(const std::string &obj_path, TextureContainer *textures);

    void fill_intersection(Intersection* isect, int triangle_id, float min_dist, glm::vec3 const& bary) const;
};

