#include <cglib/rt/triangle_soup.h>

#include <cglib/rt/interpolate.h>
#include <cglib/rt/intersection.h>
#include <cglib/rt/material.h>
#include <cglib/rt/texture_mapping.h>

#include <cglib/core/obj_mesh.h>
#include <cglib/core/glmstream.h>
#include <cglib/core/assert.h>

#include <unordered_map>

using uint = unsigned int;

TriangleSoup::
TriangleSoup(std::vector<glm::vec3>&& vertices_,
		     std::vector<glm::vec3>&& normals_,
			 std::vector<glm::vec2>&& tex_coordinates_,
			 std::vector<int>&&       material_ids_,
			 std::vector<Material>&&  materials_) :
	vertices(vertices_),
	normals(normals_),
	tex_coordinates(tex_coordinates_),
	material_ids(material_ids_),
	materials(materials_),
	num_triangles(vertices.size() / 3)
{
}

TriangleSoup::
TriangleSoup(const std::string &obj_path, TextureContainer *textures)
{
    bool verbose = false;
	OBJFile obj(verbose);
	obj.loadFile(obj_path);

	cg_assert(obj.getModelCount() > 0);

	num_triangles = obj.getFaceCount();
	if (verbose) std::cout << "obj file contains " << num_triangles << " faces" << std::endl;

	vertices.reserve(num_triangles * 3);
	normals.reserve(num_triangles * 3);
	tex_coordinates.reserve(num_triangles * 3);
	material_ids.reserve(num_triangles);

	if (verbose) std::cout << "loading faces" << std::endl;
	for(uint i = 0; i < obj.getModelCount(); i++) {
		auto m = obj.getModel(i);
		if (verbose) std::cout << "obj model name: " << m->getName() << std::endl;
		auto s = m->getSurfaces();
		if (verbose) std::cout << "num surfaces: " << s.size() << std::endl;
		for(uint j = 0; j < s.size(); j++) {
			cg_assert(s[j]);
			s[j]->appendToVertexList(*m, vertices);
			s[j]->appendToNormalList(*m, normals);
			s[j]->appendToTexcoordList(*m, tex_coordinates);

			materials.emplace_back();
			auto &mat = materials.back();
			auto &obj_mat = *(s[j]->material);

			// --- diffuse
			auto it = obj_mat.additionalInfo.find("map_Kd");
			if(textures && it != obj_mat.additionalInfo.end()) {
				std::string texturePath = it->second;

                if (textures->find(texturePath) == textures->end()) {
                    if (verbose) std::cout << "create texture: " << texturePath << std::endl;
                    textures->insert({texturePath, std::make_shared<ImageTexture>(texturePath, NEAREST, REPEAT)});
                }

				mat.k_d = (*textures)[texturePath];
			}
			else {
				mat.k_d = std::make_shared<ConstTexture>(obj_mat.diffuse);
			}
			
			// --- specular
			it = obj_mat.additionalInfo.find("map_Ks");
			if(textures && it != obj_mat.additionalInfo.end()) {
				std::string texturePath = it->second;

                if (textures->find(texturePath) == textures->end()) {
                    if (verbose) std::cout << "create texture: " << texturePath << std::endl;
                    textures->insert({texturePath, std::make_shared<ImageTexture>(texturePath, NEAREST, REPEAT)});
                }

				mat.k_s = (*textures)[texturePath];
			}
			else {
				mat.k_s = std::make_shared<ConstTexture>(obj_mat.specular);
			}

			mat.n = obj_mat.shininess;

			for(uint k = 0; k < s[j]->getFaceCount(); k++)
				material_ids.push_back(materials.size() - 1);
			/* no texture coordinates in OBJ, fall back to zero mapping */
			if(tex_coordinates.size() < vertices.size()) {
				for(uint k = 0; k < s[j]->getFaceCount() * 3; k++)
					tex_coordinates.emplace_back(0.0f);
			}
		}
		if (verbose) std::cout << "done model" << std::endl;
	}
    if (verbose) std::cout << "loading faces done" << std::endl;

	if (verbose) std::cout << vertices.size() << " vertices" << std::endl;
	if (verbose) std::cout << normals.size() << " normals" << std::endl;
	if (verbose) std::cout << tex_coordinates.size() << " texcoords" << std::endl;

	cg_assert(vertices.size() == normals.size());
	cg_assert(vertices.size() == tex_coordinates.size());
	cg_assert(vertices.size() % 3 == 0);

	num_triangles = vertices.size() / 3;
	
    cg_assert(material_ids.size() == uint32_t(num_triangles));
}

void TriangleSoup::
fill_intersection(
		Intersection* isect,
		int triangle_id,
		float min_dist,
		glm::vec3 const& bary) const
{
    cg_assert(isect);
    cg_assert(triangle_id >= 0);
    cg_assert(triangle_id < num_triangles);

    isect->t = min_dist;
    isect->primitive_id = triangle_id;
    isect->position = interpolate_barycentric(
        vertices[3 * triangle_id + 0],
        vertices[3 * triangle_id + 1],
        vertices[3 * triangle_id + 2],
        bary);
	isect->geometric_normal = glm::normalize(glm::cross(
			vertices[3 * triangle_id + 1] - vertices[3 * triangle_id + 0],
			vertices[3 * triangle_id + 2] - vertices[3 * triangle_id + 0]
		));
    isect->normal = glm::normalize(interpolate_barycentric(
        normals[3 * triangle_id + 0],
        normals[3 * triangle_id + 1],
        normals[3 * triangle_id + 2],
        bary));
	isect->shading_normal = isect->normal;
	isect->uv = interpolate_barycentric(
		tex_coordinates[3 * triangle_id + 0],
		tex_coordinates[3 * triangle_id + 1],
		tex_coordinates[3 * triangle_id + 2],
		bary);

    cg_assert(uint32_t(material_ids[triangle_id]) < materials.size());
}
