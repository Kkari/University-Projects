#include <cglib/rt/renderer.h>

#include <cglib/rt/epsilon.h>
#include <cglib/rt/intersection.h>
#include <cglib/rt/object.h>
#include <cglib/rt/light.h>
#include <cglib/rt/ray.h>
#include <cglib/rt/raytracing_context.h>
#include <cglib/rt/render_data.h>
#include <cglib/rt/scene.h>
#include <exception>
#include <stdexcept>

#include <cglib/core/thread_local_data.h>


glm::vec3 reflect(glm::vec3 const& v, glm::vec3 const& n)
{
    return glm::normalize(glm::reflect(-v, n));
}

bool refract(glm::vec3 const& v, glm::vec3 n, float eta, glm::vec3* t)
{
	cg_assert(t);
	cg_assert(std::fabs(glm::length(n) - 1.f) < EPSILON);
	cg_assert(std::fabs(glm::length(v) - 1.f) < EPSILON);

    float c = glm::dot(v, n);
    if (c < 0.f) {
        n = -n;
        c = -c;
    }
	else {
        eta = 1.f / eta;
	}

    const float radicand = 1.f-eta*eta*(1.f-c*c);
	if (radicand < 0.f) {
		// cannot compute refraction ray due to total internal reflection
		*t = glm::vec3(0.f);
		return false;
	}

    *t = glm::normalize((eta*c - std::sqrt(radicand)) * n - eta * v);
	return true;
}

float fresnel(glm::vec3 const& v, glm::vec3 const& n, float eta)
{
	cg_assert(std::fabs(glm::length(n) - 1.f) < EPSILON);
	cg_assert(std::fabs(glm::length(v) - 1.f) < EPSILON);

	if (eta == 1) {
		return 0.0f;
	}

	const float cosThetaI = glm::dot(v, n);
	/* Using Snell's law, calculate the squared sine of the
	   angle between the normal and the transmitted ray */
	float scale = (cosThetaI > 0.f) ? 1.f/eta : eta;
	float cosThetaTSqr = 1.f - (1.f-cosThetaI*cosThetaI) * (scale*scale);

	/* Check for total internal reflection */
	if (cosThetaTSqr <= 0.0f) {
		return 1.0f;
	}

	/* Find the absolute cosines of the incident/transmitted rays */
	float absCosThetaI = std::fabs(cosThetaI);
	float absCosThetaT = std::sqrt(cosThetaTSqr);

	const float Rs = (absCosThetaI - eta * absCosThetaT)
			 / (absCosThetaI + eta * absCosThetaT);
	const float Rp = (eta * absCosThetaI - absCosThetaT)
			 / (eta * absCosThetaI + absCosThetaT);

	/* No polarization -- return the unpolarized reflectance */
	return 0.5f * (Rs * Rs + Rp * Rp);
}

Ray createPrimaryRay(RenderData& data, float x, float y)
{
    const float height = static_cast<float>(data.context.params.image_height);
    const float width = static_cast<float>(data.context.params.image_width);
    const glm::vec4 origin_view_space(0.f, 0.f, 0.f, 1.f);
    const glm::vec4 origin_world_space = data.context.scene->camera->get_inverse_view_matrix(data.camera_mode) * origin_view_space;

	const float z = height/(std::tan(float(M_PI)/180.f*data.context.params.fovy));
    const glm::vec4 direction_view_space(glm::normalize(glm::vec3(
        (x - width/2.f), y - height/2.f, -z)), 0.f);
    const glm::vec4 direction_world_space = data.context.scene->camera->get_inverse_view_matrix(data.camera_mode) * direction_view_space;

    return Ray(glm::vec3(origin_world_space), glm::vec3(direction_world_space));
}

bool visible(
	RenderData &data,
	glm::vec3 const& from,
	glm::vec3 const& to)
{
	data.num_cast_rays++;
    const glm::vec3 d = glm::normalize(to-from);
    const float dist = glm::length(to-from) - 2.f*data.context.params.ray_epsilon;
    Ray ray_eps(from + data.context.params.ray_epsilon * d, d);
    for (auto& o : data.context.scene->objects) {
        cg_assert(o);
        Intersection isect;
        if (o->intersect(ray_eps, &isect) && isect.t < dist) {
            return false;
        }
    }
    return true;
}

bool shoot_ray(RenderData &data, Ray const& ray, Intersection* isect)
{
    Object* object = nullptr;

    cg_assert(isect);

    bool found_intersection = false;
    for (auto& o : data.context.scene->objects) {
        cg_assert(o);
        Intersection isect_temp;
        if (o->intersect(ray, &isect_temp) && isect_temp.t < isect->t) {
            found_intersection = true;
            object = o.get();
            *isect = isect_temp;
        }
    }

    if(found_intersection) {
        cg_assert(object);
        object->compute_shading_info(isect);
        return true;
    }

    return false;
}

bool shoot_ray(
	RenderData &data,
	Ray const& ray,
	const Ray corner_rays[4],
	Intersection* isect)
{
    Object* object = nullptr;

    cg_assert(isect);
    Ray ray_eps(ray.origin, ray.direction);

    bool found_intersection = false;
    for (auto& o : data.context.scene->objects) {
        cg_assert(o);
        Intersection isect_temp;
        if (o->intersect(ray_eps, &isect_temp) && isect_temp.t < isect->t) {
            found_intersection = true;
            object = o.get();
            *isect = isect_temp;
        }
    }

    if(found_intersection) {
        cg_assert(object);
        object->compute_shading_info(corner_rays, isect);
        return true;
    }

    return false;
}

glm::vec3 evaluate_ambient(
	RenderData &data,			// class containing raytracing information
	MaterialSample const& mat,	// the material at position
	glm::vec3 const& P,			// world space position
	glm::vec3 const& N,			// normal at the position (already normalized)
	glm::vec3 const& V)			// view vector (already normalized)
{
	cg_assert(std::fabs(glm::length(N) - 1.f) < EPSILON);
	cg_assert(std::fabs(glm::length(V) - 1.f) < EPSILON);

	glm::vec3 contribution(0.f);

	// iterate over lights and sum up their contribution
	for (auto& light_uptr : data.context.scene->lights) 
	{
		// TODO: calculate the (normalized) direction to the light
		const Light *light = light_uptr.get();
		glm::vec3 L(0.0f, 1.0f, 0.0f);
		glm::vec3 ambient = data.context.params.ambient ? mat.k_a : glm::vec3(0.0f);
		contribution += ambient * light->getEmission(-L);
	}

	return contribution;
}

glm::vec3 evaluate_phong(
	RenderData &data,			// class containing raytracing information
	MaterialSample const& mat,	// the material at position
	glm::vec3 const& P,			// world space position
	glm::vec3 const& N,			// normal at the position (already normalized)
	glm::vec3 const& V)			// view vector (already normalized)
{
	cg_assert(std::fabs(glm::length(N) - 1.f) < EPSILON);
	cg_assert(std::fabs(glm::length(V) - 1.f) < EPSILON);

	glm::vec3 contribution(0.f);
	// iterate over lights and sum up their contribution
	for (auto& light : data.context.scene->lights) {
		// TODO: calculate the (normalized) direction to the light
		const glm::vec3 L = glm::normalize(light->getPosition() - P);

		float visibility = 1.f;
		if (data.context.params.shadows) {
			// TODO: check if light source is visible
			if (!visible(data, P, light->getPosition())) {
				visibility = 0.f;
			}
		}

		glm::vec3 diffuse(0.f);
		if (data.context.params.diffuse) {
			// TODO: compute diffuse component of phong model
			if (visibility > 0.f) {
				diffuse = std::max(0.f, glm::dot(N, L)) * mat.k_d;
			}
		}

		glm::vec3 specular(0.f);
		if (data.context.params.specular) {
			// TODO: compute specular component of phong model
			if ((visibility > 0.f) && (glm::dot(L, N) > 0.f)) {
				const glm::vec3 R = reflect(L, N);
				specular = std::pow(std::max(0.f, glm::dot(R, V)), mat.n) * mat.k_s;
			}
		}

		glm::vec3 ambient = data.context.params.ambient ? mat.k_a : glm::vec3(0.0f);

		// TODO: modify this and implement the phong model as specified on the exercise sheet
		const float dist = glm::length(light->getPosition() - P);
		contribution += (visibility * (diffuse + specular) + ambient) * light->getEmission(-L) / (dist*dist);
	}

	return contribution;
}

glm::vec3 evaluate_reflection(
	RenderData & data,
	int depth,
	glm::vec3 const& P, // world space position
	glm::vec3 const& N, // Normal (already normalized)
	glm::vec3 const& V) // View vector (already normalized)
{
	// TODO: calculate reflective contribution by contructing and shooting a reflection ray.
	const glm::vec3 R = reflect(V, N);
	Ray ray_reflection(P + data.context.params.ray_epsilon * R, R);
	return trace_recursive(data, ray_reflection, depth + 1);
}

glm::vec3 evaluate_transmission(
	RenderData & data,
	int depth,          // recursion depth
	glm::vec3 const& P, // world space position
	glm::vec3 const& N, // Normal (already normalized)
	glm::vec3 const& V, // View vector (already normalized)
	float eta)
{
	// TODO: calculate transmissive contribution by constructing and shooting a transmission ray.
	glm::vec3 contribution(0.f);
	glm::vec3 T;
	if (refract(V, N, eta, &T))
	{
		Ray ray_transmission(P + data.context.params.ray_epsilon * T, T);
		contribution = trace_recursive(data, ray_transmission, depth + 1);
	}
	return contribution;
}

glm::vec3 handle_transmissive_material_single_ior(
	RenderData &data,			// class containing raytracing information
	int depth,					// the current recursion depth
	glm::vec3 const& P,			// world space position
	glm::vec3 const& N,			// normal at the position (already normalized)
	glm::vec3 const& V,			// view vector (already normalized)
	float eta)					// the relative refraction index
{
	if (data.context.params.fresnel) {
		// TODO: implement fresnel handling here.
		const float F = fresnel(V, N, eta);

		cg_assert(F >= 0.f);
		cg_assert(F <= 1.f);

		return     	  F * evaluate_reflection(data, depth, P, N, V)
			+ (1.f - F) * evaluate_transmission(data, depth, P, N, V, eta);
	}
	else {
		// just regular transmission
		return evaluate_transmission(data, depth, P, N, V, eta);
	}
}


glm::vec3 handle_transmissive_material(
	RenderData & data,
	int depth,          // recursion depth
	glm::vec3 const& P, // world space position
	glm::vec3 const& N, // Normal (already normalized)
	glm::vec3 const& V, // View vector (already normalized)
	glm::vec3 const& eta_of_channel)
{
	if (data.context.params.dispersion && !(eta_of_channel[0] == eta_of_channel[1] && eta_of_channel[0] == eta_of_channel[2])) {
		// TODO: split ray into 3 rays (one for each color channel) and implement dispersion here
		glm::vec3 contribution(0.f);
		for (int i = 0; i < 3; ++i) {
			float eta = eta_of_channel[i];
			contribution[i] += handle_transmissive_material_single_ior(data, depth, P, N, V, eta)[i];
		}
		return contribution;
	}
	else {
		const float eta = 1.f/3.f*(eta_of_channel[0]+eta_of_channel[1]+eta_of_channel[2]);
		return handle_transmissive_material_single_ior(data, depth, P, N, V, eta);
	}
	return glm::vec3(0.f);
}

glm::vec3
env_map_lookup(RenderData &data, const glm::vec3 &dir)
{
	cg_assert(std::fabs(glm::length(dir) - 1.f) < EPSILON);

	auto &env_map = data.context.scene->env_map;

	if(!env_map)
		return glm::vec3(0.0f);

	float x = (std::atan2(dir.z, dir.x) + static_cast<float>(M_PI)) / (2.0f * static_cast<float>(M_PI));
	float y = (std::asin(dir.y) + static_cast<float>(M_PI)/2.0f) / static_cast<float>(M_PI);

	switch(data.context.params.tex_filter_mode) {
	case TextureFilterMode::NEAREST:
		return glm::vec3(env_map->evaluate_nearest(0, glm::vec2(x, y)));
	default:
		return glm::vec3(env_map->evaluate_bilinear(0, glm::vec2(x, y)));
	}
}

glm::vec3 trace_recursive(RenderData & data, Ray const& ray, int depth)
{
    if (depth > data.context.params.max_depth) {
        return glm::vec3(0.f);
    }

    glm::vec3 contribution(0.f);
    Intersection isect;

	bool found_intersection = false;
    if ((   data.context.params.tex_filter_mode == TextureFilterMode::TRILINEAR
	     || data.context.params.tex_filter_mode == TextureFilterMode::DEBUG_MIP)
		&& depth == 0)
	{
        // shoot ray and compute pixel footprint with corner rays
        Ray rays[] = { createPrimaryRay(data, (data.x - 0.5f), (data.y - 0.5f)),
                       createPrimaryRay(data, (data.x + 0.5f), (data.y + 0.5f)),
                       createPrimaryRay(data, (data.x - 0.5f), (data.y + 0.5f)),
                       createPrimaryRay(data, (data.x + 0.5f), (data.y - 0.5f))};
        found_intersection = shoot_ray(data, ray, rays, &isect);
    }
    else {
        found_intersection = shoot_ray(data, ray, &isect);
    }

	if(!found_intersection) {
		return env_map_lookup(data, ray.direction);
	}

    if(depth == 0)
		data.isect = isect;

    MaterialSample mat = isect.material;
	if (data.context.params.diffuse_white_mode) {
		mat.k_a = glm::vec3(0.1f);
		mat.k_d = glm::vec3(1.0f);
		mat.k_s = glm::vec3(0.0f);
		mat.k_r = glm::vec3(0.0f);
		mat.k_t = glm::vec3(0.0f);
	}
    const glm::vec3 N = data.context.params.normal_mapping ? isect.shading_normal : isect.normal;
    const glm::vec3 V = -ray.direction;
    const bool hit_backside = glm::dot(isect.geometric_normal, V) < 0.f;

    if (!hit_backside) {
		contribution = evaluate_phong(data, mat, isect.position, N, V);
    }

    // recursive tracing
    if (!hit_backside && data.context.params.reflection && glm::length(mat.k_r) > 0.f) {
		contribution += mat.k_r * evaluate_reflection(data, depth, isect.position, N, V);
    }
    if (data.context.params.transmission && glm::length(mat.k_t) > 0.f) {
		contribution += mat.k_t * handle_transmissive_material(data, depth, isect.position, N, V, mat.eta);
    }

    return contribution;
}

