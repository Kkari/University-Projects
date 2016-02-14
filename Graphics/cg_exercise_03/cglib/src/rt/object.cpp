#include <cglib/rt/object.h>

Object::Object() :
	material(new Material()),
	texture_mapping(new ZeroMapping())
{
	material->k_d = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.18f)));
}

void Object::
compute_shading_info(Intersection* isect)
{
	cg_assert(isect);
	Intersection isect_local = transform_intersection(*isect, transform_world_to_object, transform_world_to_object_normal);
	texture_mapping->compute_tangent_space(&isect_local);

	isect_local.uv = get_uv(isect_local);
	isect_local.material.evaluate(*material, isect_local);
	isect_local.shading_normal = transform_direction_to_object_space(isect_local.material.normal,
		isect_local.normal, isect_local.tangent, isect_local.bitangent);

	*isect = transform_intersection(isect_local, transform_object_to_world, transform_object_to_world_normal);
}

void Object::
compute_shading_info(const Ray rays[4], Intersection* isect)
{
	cg_assert(isect);
	Intersection isect_local = transform_intersection(*isect, transform_world_to_object, transform_world_to_object_normal);
	texture_mapping->compute_tangent_space(&isect_local);
	isect_local.uv = get_uv(isect_local);

	Ray rays_local[4];
	for (int i = 0; i < 4; ++i) {
		rays_local[i] = transform_ray(rays[i], transform_world_to_object);
	}
	isect_local.dudv = compute_uv_aabb_size(rays_local, isect_local);
	isect_local.material.evaluate(*material, isect_local);
	isect_local.shading_normal = transform_direction_to_object_space(isect_local.material.normal,
		isect_local.normal, isect_local.tangent, isect_local.bitangent);

	*isect = transform_intersection(isect_local, transform_object_to_world, transform_object_to_world_normal);
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

void Object::
set_transform_object_to_world(glm::mat4 const& T)
{
	transform_object_to_world = T;

	transform_world_to_object = glm::inverse(T);
	transform_object_to_world_normal = glm::transpose(transform_world_to_object);
	transform_world_to_object_normal = glm::transpose(transform_object_to_world);
}

std::unique_ptr<Object> create_sphere(
		glm::vec3 const& center,
		float radius,
		glm::vec2 const& scale_uv)
{
	std::unique_ptr<Object> object(new Object());
	object->geo = std::make_shared<Sphere>(center, radius);
	object->texture_mapping = std::make_shared<SphericalMapping>(center, scale_uv);
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
	object->texture_mapping = std::make_shared<PlanarMapping>(
			center, normal, tangent, bitangent, scale_uv);
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
	object->texture_mapping = std::make_shared<PlanarMapping>(
			center-0.5f*e0-0.5f*e1, normal, e0, e1, scale_uv);
	return object;
}
