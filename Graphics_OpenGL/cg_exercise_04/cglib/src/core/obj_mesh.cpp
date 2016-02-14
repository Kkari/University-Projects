#include <cglib/core/obj_mesh.h>
#include <cglib/core/assert.h>

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

OBJMaterial::OBJMaterial(const std::string& name_) :
    name(name_),
    diffuse(0.8f),
    ambient(0.2f),
    specular(0.0f)
{

}

uint32_t OBJSurface::getFaceCount() const {
	return vertexIndices.size();
}

uint32_t OBJModel::getFaceCount() const {
	uint32_t sum = 0;
	for(auto i: m_surfaces)
		sum += i->getFaceCount();
	return sum;
}

uint32_t OBJFile::getFaceCount() const {
	uint32_t sum = 0;
	for(auto i: m_models)
		sum += i.second->getFaceCount();
	return sum;
}

void OBJSurface::buildVertexList(const OBJModel& model, std::vector<glm::vec3>& target) const {
	target.clear();
	target.reserve(vertexIndices.size() * 3);
	appendToVertexList(model, target);
}

void OBJSurface::appendToVertexList(
		const OBJModel& model, std::vector<glm::vec3>& target) const {
	const std::vector<glm::vec3>& vertices = model.getVertices();

	for (const glm::uvec3 &index : vertexIndices)
	{
		glm::vec3 v0, v1, v2;

		v0 = vertices.at(index.x);
		v1 = vertices.at(index.y);
		v2 = vertices.at(index.z);

		target.push_back(v0);
		target.push_back(v1);
		target.push_back(v2);
	}
}

void OBJSurface::buildNormalList(const OBJModel& model, std::vector<glm::vec3>& target) const {
	target.clear();
	target.reserve(normalIndices.size() * 3);
	appendToNormalList(model, target);
}

void OBJSurface::appendToNormalList(
		const OBJModel& model, std::vector<glm::vec3>& target) const {
	const std::vector<glm::vec3>& normals = model.getNormals();

	for (const glm::uvec3 index : normalIndices)
	{
		glm::vec3 n0, n1, n2;

		n0 = normals.at(index.x);
		n1 = normals.at(index.y);
		n2 = normals.at(index.z);

		target.push_back(n0);
		target.push_back(n1);
		target.push_back(n2);
	}
}

void OBJSurface::buildTexcoordList(const OBJModel& model, std::vector<glm::vec2>& target) const {
	target.clear();
	target.reserve(texcoordIndices.size() * 3);
	appendToTexcoordList(model, target);
}

void OBJSurface::appendToTexcoordList(const OBJModel& model, std::vector<glm::vec2>& target) const {
	const std::vector<glm::vec2>& texcoords = model.getTexcoords();

	for (const glm::uvec3 index : texcoordIndices)
	{
		glm::vec2 t0, t1, t2;

		t0 = texcoords[index.x];
		t1 = texcoords[index.y];
		t2 = texcoords[index.z];

		target.push_back(t0);
		target.push_back(t1);
		target.push_back(t2);
	}
}

OBJModel::OBJModel(const std::string& name) :
	m_name(name),
	m_vertices(),
	m_normals(),
	m_texcoords()
{

}

void OBJModel::scale(const glm::vec3& scale) {
	for (size_t i = 0; i < m_vertices.size(); ++i) {
		m_vertices[i] *= scale;
	}
}

void OBJModel::scale(float scale) {
	this->scale(glm::vec3(scale));
}

void OBJModel::translate(const glm::vec3& vec) {
	for (size_t i = 0; i < m_vertices.size(); ++i) {
		m_vertices[i] = m_vertices[i] + vec;
	}
}

void OBJModel::getAABB(glm::vec3& minEdge, glm::vec3& maxEdge) const {
	if (m_vertices.size() == 0) {
		minEdge = maxEdge = glm::vec3(0);
		return;
	}

	minEdge = glm::vec3(1e37f);
	maxEdge = glm::vec3(-1e37f);

	// Search for min and max edge
	for (size_t i = 0; i < m_vertices.size(); ++i) {
		const glm::vec3& v = m_vertices[i];

		minEdge.x = std::min(minEdge.x, v.x);
		minEdge.y = std::min(minEdge.y, v.y);
		minEdge.z = std::min(minEdge.z, v.z);

		maxEdge.x = std::max(maxEdge.x, v.x);
		maxEdge.y = std::max(maxEdge.y, v.y);
		maxEdge.z = std::max(maxEdge.z, v.z);
	}
}

const std::string& OBJModel::getName() const {
	return m_name;
}

const std::vector<glm::vec3>& OBJModel::getVertices() const {
    return m_vertices;
}

std::vector<glm::vec3>& OBJModel::getVertices() {
    return m_vertices;
}

const std::vector<glm::vec3>& OBJModel::getNormals() const {
    return m_normals;
}

std::vector<glm::vec3>& OBJModel::getNormals() {
    return m_normals;
}

const std::vector<glm::vec2>& OBJModel::getTexcoords() const {
    return m_texcoords;
}

std::vector<glm::vec2>& OBJModel::getTexcoords() {
    return m_texcoords;
}

const std::vector<std::shared_ptr<OBJSurface>>& OBJModel::getSurfaces() const {
	for(auto i: m_surfaces) {
		cg_assert(i);
	}
	return m_surfaces;
}

std::shared_ptr<OBJSurface> OBJModel::addSurface(const OBJFile& objFile, uint32_t materialIndex) {
	auto surf = std::make_shared<OBJSurface>();
    m_surfaces.push_back(surf);

    surf->material = objFile.getMaterial(materialIndex);

    // Optimize alloc of vector
    surf->vertexIndices.reserve(32);
    surf->normalIndices.reserve(32);
    surf->texcoordIndices.reserve(32);

    return surf;
}

OBJFile::OBJFile(bool verbose_, bool try_triangulate_quads_) :
	m_models(),
	m_material(),
	verbose(verbose_),
	try_triangulate_quads(try_triangulate_quads_)
{
}

OBJFile::~OBJFile()
{
	//Everything is stored in the std::map and std::vector, so it gets removed
}

namespace
{

/**
* Gets the path of a file.
* Example: "/home/test/file.obj" -> "/home/test/"
*/
std::string getFilePath(const std::string& filename) {
    size_t pos = filename.rfind("/");

    if (pos == std::string::npos)
        return "";

    return filename.substr(0, pos + 1);
}

/**
* Expects input of %d/%d/%d where / and %d can miss.
* Writes the parsed data in the result int array.
* Set true in rSet array, if the value was in the word.
*/
inline const char* parseHelper(const char* cursor, const char* endCursor, uint32_t (&result)[3], bool (&rSet)[3])
{
	result[0] = result[1] = result[2] = 0;
	rSet[0] = rSet[1] = rSet[2] = false;

	char* numEnd = (char*) cursor;
	result[0] = (uint32_t) strtol(cursor, &numEnd, 10);
	if (numEnd <= endCursor) {
		rSet[0] = (cursor < numEnd);
		cursor = numEnd;
	}

	if (*cursor == '/') {
		numEnd = (char*) ++cursor;
		result[1] = (uint32_t) strtol(cursor, &numEnd, 10);
		if (numEnd <= endCursor) {
			rSet[1] = (cursor < numEnd);
			cursor = numEnd;
		}

		if (*cursor == '/') {
			numEnd = (char*) ++cursor;
			result[2] = (uint32_t) strtol(cursor, &numEnd, 10);
			if (numEnd <= endCursor) {
				rSet[2] = (cursor < numEnd);
				cursor = numEnd;
			}
		}
	}

	return cursor;
}

template <class It>
inline It skipws(It cursor, It end)
{
	while (cursor < end && isspace(*cursor))
		++cursor;
	return cursor;
}

template <class It>
inline It nextws(It cursor, It end)
{
	while (cursor < end && !isspace(*cursor))
		++cursor;
	return cursor;
}

template <class It>
inline It nextword(It cursor, It end)
{
	return skipws(nextws(cursor, end), end);
}

template <class It>
inline float extract_float(It& cursor, It end)
{
	char* numEnd = (char*) cursor;
	auto f = (float) strtod(cursor, &numEnd);
	if (numEnd <= end) cursor = numEnd;
	return f;
}

template <class It>
inline glm::vec2 extract_vec2(It& cursor, It end)
{
	glm::vec2 r;
	r.x = extract_float(cursor, end);
	r.y = extract_float(cursor, end);
	return r;
}

template <class It>
inline glm::vec3 extract_vec3(It& cursor, It end)
{
	glm::vec3 r;
	r.x = extract_float(cursor, end);
	r.y = extract_float(cursor, end);
	r.z = extract_float(cursor, end);
	return r;
}

} // namespace

bool OBJFile::loadFile(const std::string& filename, bool abortOnMissingMaterial) {
	if (verbose) {
		std::cout << "Loading OBJ file '" << filename.c_str() << "'" << std::endl;
	}

	std::ifstream f(filename, std::ios::binary);

	if (!f.is_open()) {
		std::cerr << "could not open file '" << filename << "'" << std::endl;
		return false;
	}

	std::string line;

	std::shared_ptr<OBJModel> currentModel = 0;
	std::shared_ptr<OBJSurface> currentSurface = 0;
	uint32_t currentMaterialIndex = 0;
	int currentVertexOffset = 1;
	int currentTexcoordOffset = 1;
	int currentNormalOffset = 1;

	while (getline(f, line))
	{
		auto* cursor = line.data();
		auto* endCursor = cursor + line.size();

		// remove '\r' on line end
		if (cursor < endCursor && endCursor[-1] == '\r') {
			--endCursor;
		}

		cursor = skipws(cursor, endCursor);

		// skip empty lines
		if (cursor == endCursor) {
			continue;
		}

		switch (*cursor)
		{
			case '#':
				break;
			case 'o': {
				std::string objectName(nextword(cursor, endCursor), endCursor);

				if(currentModel) {
					currentVertexOffset += currentModel->getVertices().size();
					currentTexcoordOffset += currentModel->getTexcoords().size();
					currentNormalOffset += currentModel->getNormals().size();
				}
				currentModel = addModel(objectName);
				currentSurface = 0;
				break;
			}
			case 'v': {
				++cursor;

				if (!currentModel)
					currentModel = addDefaultModel();
				
				switch (*cursor++)
				{
					case ' ':
					case '\t': {
						currentModel->getVertices().push_back(extract_vec3(cursor, endCursor));
						break;
					}
					case 'n': {
						currentModel->getNormals().push_back(extract_vec3(cursor, endCursor));
						break;
					}
					case 't': {
						currentModel->getTexcoords().push_back(extract_vec2(cursor, endCursor));
						break;
					}
				}
				break;
			}
			case 'f': {
				if (!currentSurface) {
					if (!currentModel)
						currentModel = addDefaultModel();

					if (m_material.size() == 0)
						addMaterial("default");

					currentSurface = currentModel->addSurface(*this, currentMaterialIndex);
				}

                // array of vertex, texcoord, normal
				uint32_t vtn_0[3];  bool vtn_0set[3];
				uint32_t vtn_1[3];  bool vtn_1set[3];
				uint32_t vtn_2[3];  bool vtn_2set[3];
				uint32_t vtn_3[3];  bool vtn_3set[3];

				cursor = nextword(cursor, endCursor);
				cursor = parseHelper(cursor, nextws(cursor, endCursor), vtn_0, vtn_0set);
				cursor = nextword(cursor, endCursor);
				cursor = parseHelper(cursor, nextws(cursor, endCursor), vtn_1, vtn_1set);
				cursor = nextword(cursor, endCursor);
				cursor = parseHelper(cursor, nextws(cursor, endCursor), vtn_2, vtn_2set);
				cursor = nextword(cursor, endCursor);
				cursor = parseHelper(cursor, nextws(cursor, endCursor), vtn_3, vtn_3set);

                // Expecting that vtn_0set, vtn_1set, vtn_2set are equal (others dont make sense)

                if (vtn_0set[0]) {
                    currentSurface->vertexIndices.push_back(
							glm::uvec3(vtn_0[0], vtn_1[0], vtn_2[0])
							- glm::uvec3(currentVertexOffset)
							);

					if (try_triangulate_quads && vtn_3set[0]) {
						currentSurface->vertexIndices.push_back(
							glm::uvec3(vtn_0[0], vtn_2[0], vtn_3[0])
							- glm::uvec3(currentVertexOffset)
							);
					}
				}

                if (vtn_0set[1]) {
                    currentSurface->texcoordIndices.push_back(
							glm::uvec3(vtn_0[1], vtn_1[1], vtn_2[1])
							- glm::uvec3(currentTexcoordOffset)
							);

					if (try_triangulate_quads && vtn_3set[1]) {
						currentSurface->texcoordIndices.push_back(
							glm::uvec3(vtn_0[1], vtn_2[1], vtn_3[1])
							- glm::uvec3(currentTexcoordOffset)
							);
					}
				}

                if (vtn_0set[2]) {
                    currentSurface->normalIndices.push_back(
							glm::uvec3(vtn_0[2], vtn_1[2], vtn_2[2])
							- glm::uvec3(currentNormalOffset)
							);

					if (try_triangulate_quads && vtn_3set[2]) {
						currentSurface->normalIndices.push_back(
							glm::uvec3(vtn_0[2], vtn_2[2], vtn_3[2])
							- glm::uvec3(currentNormalOffset)
							);
					}
				}

                break;
			}
			case 'm':
            {
				auto endofword = nextws(cursor, endCursor);
				if (std::string(cursor, endofword) != "mtllib")
					break;
				cursor = skipws(endofword, endCursor);

                std::string matFile(cursor, endCursor);

                // Create relativ Path to this .obj file
                matFile = getFilePath(filename) + matFile;

                // Load the material file
                bool success = loadMaterialFile(matFile);

				if (verbose && !success)
					printf("Failed to load material file '%s'\n", matFile.c_str());

                if (!success && abortOnMissingMaterial)
                    return false;

                break;
            }

            case 'u':
            {
				auto endofword = nextws(cursor, endCursor);
				if (std::string(cursor, endofword) != "usemtl")
					break;
				cursor = skipws(endofword, endCursor);

				std::string matName(cursor, endCursor);

				int32_t matIndex = findMaterial(matName);

				if (verbose && matIndex == -1)
					printf("Unable to find material '%s'\n", matName.c_str());

				if (matIndex == -1)
					break;

				currentMaterialIndex = matIndex;
            }

		}
	}

	if (verbose)
	{
		printf("Finished loading '%s'; Stats:\n", filename.c_str());
		printf("%u Models\n%u Materials\n", getModelCount(), getMaterialCount());
	}

	return true;
}

bool OBJFile::loadMaterialFile(const std::string& filename) {
	if (verbose)
		printf("Loading material file '%s'\n", filename.c_str());

    std::ifstream f(filename, std::ios::binary);

	if (!f.is_open())
		return false;

	std::string line;

	std::shared_ptr<OBJMaterial> currentMaterial = 0;

	while (getline(f, line))
	{
		// remove '\r' on line end
		if (line.size() > 0 && line[line.size() - 1] == '\r')
		{
			line.erase(line.size() - 1);
		}

		std::istringstream ss(line);

		std::string word;
		ss >> word;

		if (word.size() == 0)
            continue;

        switch (word[0])
        {
            case '#':
                break;

            case 'n': { // New material
                if (word != "newmtl")
                    break;

                std::string name;   ss >> name;

				if (verbose)
					printf("Reading material: '%s'\n", name.c_str());

                currentMaterial = addMaterial(name);
                break;
            }
            case 'N': { // Shininess
                if (!currentMaterial)
                    currentMaterial = addDefaultMaterial();

                if (word == "Ns")
                {
					ss >> currentMaterial->shininess;
                }
                // wavefront shininess is from [0, 1000] -> scale for OpenGL
                //currentMaterial->shininess /= 1000.0f;
                //currentMaterial->shininess *= 128.0f;

                break;
            }
            case 'K': {
                if (!currentMaterial)
                    currentMaterial = addDefaultMaterial();

                if (word == "Kd")
                {
                    ss >> currentMaterial->diffuse[0];
                    ss >> currentMaterial->diffuse[1];
                    ss >> currentMaterial->diffuse[2];
                }
                else if (word == "Ks")
                {
                    ss >> currentMaterial->specular[0];
                    ss >> currentMaterial->specular[1];
                    ss >> currentMaterial->specular[2];
                }
                else if (word == "Ka")
                {
                    ss >> currentMaterial->ambient[0];
                    ss >> currentMaterial->ambient[1];
                    ss >> currentMaterial->ambient[2];
                }
                break;
            }

            default: {
				// if there are 2 or more words, write them in additional info map
				std::string value;
				
				if (ss.tellg() > 1)
				{
					value = line.substr(static_cast<std::size_t>(1 + ss.tellg()));
				}

				// when word is map_*, value is a path. replace \ in path by /
				std::string map_prefix = "map_";
				if(std::mismatch(map_prefix.begin(), map_prefix.end(), word.begin()).first
					== map_prefix.end()) {
					std::replace(value.begin(), value.end(), '\\', '/');
				}

				if (verbose)
					printf("additional material info: '%s' = '%s'\n", word.c_str(), value.c_str());

				if (!currentMaterial)
                    currentMaterial = addDefaultMaterial();

				currentMaterial->additionalInfo.insert(std::make_pair(word, value));
				break;
            }
        }
	}

	return true;
}

std::shared_ptr<const OBJModel> OBJFile::getModel(size_t index) const {
	cg_assert(index < m_models.size());

	auto iter = m_models.begin();

	for (size_t i = 0; i < index; ++i)
        ++iter;

	return (iter->second);
}

std::shared_ptr<OBJModel> OBJFile::getModel(size_t index) {
	cg_assert(index < m_models.size());

	auto iter = m_models.begin();

	for (size_t i = 0; i < index; ++i)
        ++iter;

	return (iter->second);
}


std::shared_ptr<const OBJModel> OBJFile::getModel(const std::string& name) const {
    return (m_models.at(name));
}

std::shared_ptr<OBJModel> OBJFile::getModel(const std::string& name) {
    return (m_models.at(name));
}

uint32_t OBJFile::getModelCount() const {
	return m_models.size();
}

std::shared_ptr<const OBJMaterial> OBJFile::getMaterial(uint32_t index) const {
	cg_assert(index < m_material.size());

    return m_material[index];
}

std::shared_ptr<OBJMaterial> OBJFile::getMaterial(uint32_t index) {
	cg_assert(index < m_material.size());

    return m_material[index];
}

std::shared_ptr<const OBJMaterial> OBJFile::getMaterial(const std::string& name) const {
	int32_t index = findMaterial(name);

	if (index == -1)
		return 0;

	return m_material[index];
}

std::shared_ptr<OBJMaterial> OBJFile::getMaterial(const std::string& name) {
	int32_t index = findMaterial(name);

	if (index == -1)
		return 0;

	return m_material[index];
}

uint32_t OBJFile::getMaterialCount() const {
    return m_material.size();
}

std::shared_ptr<OBJModel>
OBJFile::addModel(const std::string& name) {
	auto p = std::make_shared<OBJModel>(name);
	m_models[name] = p;
	return p;
}

std::shared_ptr<OBJModel>
OBJFile::addDefaultModel() {
	return addModel("default");
}

std::shared_ptr<OBJMaterial> OBJFile::addMaterial(const std::string& name) {
	auto ptr = std::make_shared<OBJMaterial>(name);
    m_material.push_back(ptr);
	return ptr;
}

std::shared_ptr<OBJMaterial> OBJFile::addDefaultMaterial() {
    return addMaterial("default");
}

int32_t OBJFile::findMaterial(const std::string& name) const {
	// Linear search
	for (size_t i = 0; i < m_material.size(); ++i) {
		if (m_material[i]->name == name)
			return i;
	}

	return -1;
}
