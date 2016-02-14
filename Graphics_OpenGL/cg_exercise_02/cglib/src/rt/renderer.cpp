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

float max_unobstructed_distance(
	RenderData &data,
	glm::vec3 const& from,
	glm::vec3 const& dir)
{
	data.num_cast_rays++;
    Ray ray_eps(from + data.context.params.ray_epsilon * dir, dir);
	float closest_hit = FLT_MAX;
    for (auto& o : data.context.scene->objects) {
        cg_assert(o);
        Intersection isect;
        if (o->intersect(ray_eps, &isect) && isect.t < closest_hit) {
            closest_hit = isect.t + data.context.params.ray_epsilon;
        }
    }
    return closest_hit;
}

bool shoot_ray(RenderData &data, Ray const& ray, Intersection* isect)
{
    Object* object = nullptr;

    cg_assert(isect);
    Ray ray_eps(ray.origin + data.context.params.ray_epsilon * ray.direction, ray.direction);

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
        object->compute_shading_info(isect);
        return true;
    }

    return false;
}

glm::vec3 trace_recursive(RenderData & data, Ray const& ray, int depth)
{
    if (depth > data.context.params.max_depth) {
        return glm::vec3(0.f);
    }

    glm::vec3 contribution(0.f);
    Intersection isect;

	if (!shoot_ray(data, ray, &isect)) {
		return glm::vec3(0.f);
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
    const glm::vec3 N = isect.normal;
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

