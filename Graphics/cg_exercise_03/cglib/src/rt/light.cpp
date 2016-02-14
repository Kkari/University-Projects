#include <cglib/rt/light.h>
#include <cglib/rt/epsilon.h>
#include <cglib/core/assert.h>

#include <algorithm>

glm::vec3 SpotLight::getEmission(glm::vec3 const& omega) const {
	cg_assert(std::fabs(glm::length(omega) - 1.f) < EPSILON);
	const float alpha = std::max(0.f, glm::dot(omega, direction));
	return emission * std::pow(alpha, falloff);
}
