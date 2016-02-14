#include <cglib/rt/texture_mapping.h>
#include <cglib/rt/intersection.h>
#include <cglib/core/assert.h>

void TextureMapping::compute_tangent_space(Intersection* isect) const
{
	isect->normal = glm::normalize(isect->normal);
	if (1.f-std::fabs(isect->normal[1]) < 1e-4f)
	{
		isect->tangent = glm::vec3(1, 0, 0);
	}
	else
	{
		isect->tangent = -glm::normalize(glm::vec3(-isect->normal[2], 0, isect->normal[0]));
	}
	isect->bitangent = glm::normalize(glm::cross(isect->tangent, isect->normal));
}

glm::vec2 UVMapping::get_uv(Intersection const& isect) const
{
    return isect.uv;
}

void UVMapping::compute_tangent_space(Intersection* isect) const
{
	cg_assert(false && "Implement tangent space for UV mapping!");
}

glm::vec2 SphericalMapping::get_uv(Intersection const& isect) const
{
    const glm::vec3 d = glm::normalize(isect.position - center);
	float const v = 1.f-std::acos(d[1])/float(M_PI);
	float u = std::atan2(-d[2], d[0])/(2.f*float(M_PI));
	if (u < 0.f) u += 1.f;
	return scale_uv * glm::vec2(u, v);
}

glm::vec2 PlanarMapping::get_uv(Intersection const& isect) const
{
    const glm::vec3 d(isect.position-p);
    return glm::vec2(glm::dot(d, tangent), glm::dot(d, bitangent))/scale_uv;
}

void PlanarMapping::compute_tangent_space(Intersection* isect) const
{
	isect->normal    = glm::normalize(normal);
	isect->tangent   = glm::normalize(tangent);
	isect->bitangent = glm::normalize(bitangent);
}

