#pragma once
#ifndef OBJMESH_H
#define OBJMESH_H

#include <glm/glm.hpp>
#include <vector>
#include <map>
#include <string>
#include <stdint.h>
#include <memory>

/**
OBJ file model loader.

Example usage:

Load a .obj File:
	OBJ file;
	file.loadFile(path);

Get the first model:
	OBJModel& model = file.getModel(0);

Get the surfaces
	const std::vector<OBJSurface>& surfaces = model.getSurfaces();

Get the vertices from the first surface:
	std::vector<glm::vec3> vertices;
	surfaces[0].buildVertexList(model, vertices);

*/


class OBJFile;
class OBJModel;

struct OBJMaterial
{
	/// Material name
	std::string name;

	glm::vec3 diffuse;
	glm::vec3 ambient;
	glm::vec3 specular;
	glm::vec3 emmissive;
	float shininess;

	/// Additional parameters from the .mtl file
	std::map<std::string, std::string> additionalInfo;

	/// Constructor
	OBJMaterial(const std::string& name);
};

struct OBJSurface
{
	/// Ptr to the material used by this surface (stored in OBJFile)
	std::shared_ptr<const OBJMaterial> material;

	std::vector<glm::uvec3> vertexIndices;
	std::vector<glm::uvec3> normalIndices;
	std::vector<glm::uvec3> texcoordIndices;

	/// Writes all Vertices in the target vector. Vertices [0,1,2], [3,4,5], ... are triangles.
	/// Writes all normals in the target vector. The index is the same as from buildVertexList.
	/// Writes all texcoords in the target vector. The index is the same as from buildVertexList.
	void buildNormalList  (const OBJModel& model, std::vector<glm::vec3>& target) const;
	void buildVertexList  (const OBJModel& model, std::vector<glm::vec3>& target) const;
	void buildTexcoordList(const OBJModel& model, std::vector<glm::vec2>& target) const;

	/// same as functions above but do not clear the vector
	void appendToVertexList  (const OBJModel& model, std::vector<glm::vec3>& target) const;
	void appendToNormalList  (const OBJModel& model, std::vector<glm::vec3>& target) const;
	void appendToTexcoordList(const OBJModel& model, std::vector<glm::vec2>& target) const;

	uint32_t getFaceCount() const;
};

class OBJModel
{
private:
	/// Model name
	std::string m_name;

	/// Vertices
	std::vector<glm::vec3> m_vertices;

	/// Normals
	std::vector<glm::vec3> m_normals;

	/// Texture Coords
	std::vector<glm::vec2> m_texcoords;

	/// Surface info
	std::vector<std::shared_ptr<OBJSurface>> m_surfaces;

public:
	/// Constructor
	OBJModel(const std::string& name);

	/// Rescales the model with the given factor
	void scale(const glm::vec3& scale);

	/// Rescales the model with the given factor
	void scale(float scale);

	/// Translates the model
	void translate(const glm::vec3& vec);

	/// Calculates the axis aligned bounding box of the model
	/// and store the edges in the minEdge and maxEdge parameter.
	void getAABB(glm::vec3& minEdge, glm::vec3& maxEdge) const;

	/// Get the name of the model
	const std::string& getName() const;

	const std::vector<glm::vec3>& getVertices() const;
	std::vector<glm::vec3>& getVertices();

	const std::vector<glm::vec3>& getNormals() const;
	std::vector<glm::vec3>& getNormals();

	const std::vector<glm::vec2>& getTexcoords() const;
	std::vector<glm::vec2>& getTexcoords();

	const std::vector<std::shared_ptr<OBJSurface>>& getSurfaces() const;

	uint32_t getFaceCount() const;

	std::shared_ptr<OBJSurface> addSurface(const OBJFile& objFile, uint32_t materialIndex);
};

/**
* Helper Class.
* Able to load a .obj File and extract the models.
* To Remove alle loaded models from this file just call the destructor.
*/
class OBJFile
{
private:
	/// Map of all models in the obj file
	std::map<std::string, std::shared_ptr<OBJModel>> m_models;

	/// Contains all materials used in the obj file
	std::vector<std::shared_ptr<OBJMaterial>> m_material;

	/// Print info while working?
	bool verbose:1;

	/// Try triangulate vertices with 4 points?
	bool try_triangulate_quads:1;

	/// Adds an new Model and insert it into m_models, returns the pointer on it
	std::shared_ptr<OBJModel> addModel(const std::string& name);

	/// Same as addModel(), but with default name
	std::shared_ptr<OBJModel> addDefaultModel();

	/// Adds a new Material and insert it into m_material, returns the pointer on it
	std::shared_ptr<OBJMaterial> addMaterial(const std::string& name);

	/// Same as addMaterial, but with default name
	std::shared_ptr<OBJMaterial> addDefaultMaterial();

	/// Returns the matarial ID, or -1 if not found
	int32_t findMaterial(const std::string& name) const;

public:
	/// Construct a new container for loading .obj files.
	OBJFile(bool verbose = false, bool try_triangulate_quads = false);

	/// Removes all loaded models/materials
	~OBJFile();

	/// Loads a .obj file, returns true if this was successful
	bool loadFile(const std::string& filename, bool abortOnMissingMaterial = false);

	bool loadMaterialFile(const std::string& filename);

	/// Returns the reference to the model, the index must be valid
	std::shared_ptr<const OBJModel> getModel(size_t index) const;
	std::shared_ptr<OBJModel> getModel(size_t index);

	/// Returns the std::shared_ptrerence to the Model, the given name must be valid
	std::shared_ptr<const OBJModel> getModel(const std::string& name) const;
	std::shared_ptr<OBJModel> getModel(const std::string& name);

	/// Gets the count of models in the loaded .obj file.
	uint32_t getModelCount() const;

	/// Returns the ptr to the material, the index must be valid
	std::shared_ptr<const OBJMaterial> getMaterial(uint32_t index) const;
	std::shared_ptr<OBJMaterial> getMaterial(uint32_t index);

	/// Returns the ptr to the material, or 0 if not found
	std::shared_ptr<const OBJMaterial> getMaterial(const std::string& name) const;
	std::shared_ptr<OBJMaterial> getMaterial(const std::string& name);

	/// Gets the count of materials
	uint32_t getMaterialCount() const;

	uint32_t getFaceCount() const;
};

#endif
