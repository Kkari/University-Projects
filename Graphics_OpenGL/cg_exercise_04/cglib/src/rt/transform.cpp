#include <cglib/rt/transform.h>
#include <cglib/core/assert.h>

glm::vec3 transform_direction(glm::mat4 const& transform, glm::vec3 const& d)
{
	return glm::normalize(glm::vec3(transform*glm::vec4(d, 0.f)));
}

glm::vec3 transform_position(glm::mat4 const& transform, glm::vec3 const& p)
{
	return glm::vec3(transform*glm::vec4(p, 1.f));
}

glm::vec3 transform_direction_to_object_space(
	glm::vec3 const& d, 
	glm::vec3 const& normal, 
	glm::vec3 const& tangent, 
	glm::vec3 const& bitangent)
{
	cg_assert(std::fabs(glm::length(normal)    - 1.0f) < 1e-4f);
	cg_assert(std::fabs(glm::length(tangent)   - 1.0f) < 1e-4f);
	cg_assert(std::fabs(glm::length(bitangent) - 1.0f) < 1e-4f);
	glm::mat3 const tangent_to_object = glm::mat3(tangent, normal, bitangent);
	return glm::normalize(tangent_to_object * d);
}

