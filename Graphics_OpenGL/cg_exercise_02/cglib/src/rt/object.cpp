#include <cglib/rt/object.h>

Object::Object() :
	material(new Material()),
	texture_mapping(new ZeroMapping())
{
	material->k_d = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.18f)));
}

bool Object::
intersect(Ray const& ray, Intersection* isect) const
{
	return geo->intersect(ray, isect);
}

void Object::
compute_shading_info(Intersection* isect)
{
	cg_assert(isect);
	isect->uv = get_uv(*isect);
	isect->material.evaluate(*material, *isect);
}

void Object::
compute_shading_info(const Ray rays[4], Intersection* isect)
{
	cg_assert(isect);
	isect->uv       = get_uv(*isect);
	isect->material.evaluate(*material, *isect);
}

void Object::
get_intersection_uvs(glm::vec3 const positions[4], Intersection const& isect, glm::vec2 uvs[4])
{
	for (int i = 0; i < 4; ++i)
	{
		Intersection isect_corner = isect;
		isect_corner.position = positions[i];
		uvs[i] = get_uv(isect_corner);
	}

	for (int k = 0; k < 2; ++k)
	{
		glm::vec2 duvs[2];
		for (int j = 0; j < 2; ++j) {
			duvs[j] = uvs[2*k+j]-isect.uv;
		}
		if (glm::dot(duvs[0], duvs[1]) > 0) {
			int m = glm::length(duvs[0]) < glm::length(duvs[1]) ? 0 : 1;
			uvs[2*k+0] = isect.uv + duvs[m];
			uvs[2*k+1] = isect.uv - duvs[m];
		}
	}
}

glm::vec2 Object::
get_uv(Intersection const& isect)
{
	return texture_mapping->get_uv(isect);
}

std::unique_ptr<Object> create_sphere(
		glm::vec3 const& center,
		float radius,
		glm::vec2 const& scale_uv)
{
	std::unique_ptr<Object> object(new Object());
	object->geo = std::make_shared<Sphere>(center, radius);
	object->texture_mapping = std::make_shared<ZeroMapping>();
	return object;
}

std::unique_ptr<Object> create_plane(
		glm::vec3 const& center,
		glm::vec3 const& normal,
		glm::vec3 const& tangent,
		glm::vec3 const& bitangent,
		glm::vec2 const& scale_uv)
{
	cg_assert(normal == glm::normalize(glm::cross(
					glm::normalize(tangent),
					glm::normalize(bitangent))));

	std::unique_ptr<Object> object(new Object());
	object->geo = std::make_shared<Plane>(center, normal);
	object->texture_mapping = std::make_shared<ZeroMapping>();
	return object;
}

std::unique_ptr<Object> create_plane(
		glm::vec3 const& center,
		glm::vec3 const& normal)
{
	const glm::vec3 never_col(normal.y, -normal.z, -normal.x);
	const glm::vec3 tangent = glm::normalize(glm::cross(normal, never_col));
	const glm::vec3 bitangent = glm::normalize(glm::cross(tangent, normal));

	return create_plane(center, normal, tangent, bitangent);
}

std::unique_ptr<Object> create_plane(
		glm::vec3 const& center,
		glm::vec3 const& normal,
		glm::vec3 const& tangent,
		glm::vec2 const& scale_uv)
{
	const glm::vec3 bitangent = glm::normalize(glm::cross(glm::normalize(normal), glm::normalize(tangent)));
	return create_plane(center, glm::normalize(normal), glm::normalize(tangent), bitangent, scale_uv);
}

std::unique_ptr<Object> create_quad(
		glm::vec3 const& center,
		glm::vec3 const& normal,
		glm::vec3 const& e0,
		glm::vec3 const& e1,
		glm::vec2 const& scale_uv)
{
	cg_assert(normal == glm::normalize(glm::cross(
					glm::normalize(e0),
					glm::normalize(e1))));

	std::unique_ptr<Object> object(new Object());
	object->geo = std::make_shared<Quad>(center, e0, e1);
	object->texture_mapping = std::make_shared<ZeroMapping>();
	return object;
}
