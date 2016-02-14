#pragma once

#include <glm/glm.hpp>

class Object;
class Ray;
struct RenderData;
class Intersection;
struct ThreadLocalData;
class MaterialSample;

/*
 * reflect the vector v at the normal vector n. v points "away from n"
 */
glm::vec3 reflect(glm::vec3 const& v, glm::vec3 const& n);

/*
 * use this to compute the refraction direction for transmissive object.
 *
 * eta is the relative refraction index, i.e the optical thickness inside divided by the optical thickness outside the medium/object,
 * if v lies within the hemisphere of n (dot(v, n) >= 0), we are entering the object, otherwise we are leaving the object.
 *
 * The function returns false, iff there is no refraction ray due to total internal reflexion.
 */
bool refract(glm::vec3 const& v, glm::vec3 n, float eta, glm::vec3* t);

/*
 * compute the fresnel term for a direction v at a point with normal n and relative refraction index eta.
 *
 * eta is the relative refraction index, i.e the optical thickness inside divided by the optical thickness outside the medium/object,
 * if v lies within the hemisphere of n (dot(v, n) >= 0), we are entering the object, otherwise we are leaving the object.
 *
 * the function returns the fraction of energy which will be reflected.
 */
float fresnel(glm::vec3 const& v, glm::vec3 const& n, float eta);

/*
 * creates a ray starting from the camera position through the (sub-)pixel location (x,y)
 */
Ray createPrimaryRay(
	RenderData &data,
	float x, float y);

/*
 * check if a point "to" is visible from the point "from"
 */
bool visible(
	RenderData &data,
	glm::vec3 const& from,
	glm::vec3 const& to);

/*
 * Shoot a ray and return intersection information
 */
bool shoot_ray(
	RenderData &data,
	Ray const& ray,
	Intersection* isect);

/*
 * Shoot a ray and the corner rays to compute the footprint of the pixel (in uv-texture space)
 */
bool shoot_ray(
	RenderData &data,
	Ray const& ray,
	const Ray corner_rays[4],
	Intersection* isect);

/*
 *  Loops over all lights and evaluates a simple ambient lighting model
 *
 *  Parameters:
 *		- context for accessing the lights and rendering parameters.
 *		- render_data specific for one pixel
 *		- material at the location to be shaded.
 *		- P the point to be shaded.
 *		- N the surface normal at P.
 *		- V the normalized view vector (from P to the camera)
 */
glm::vec3 evaluate_ambient(
	RenderData &data,			// class containing raytracing information
	MaterialSample const& mat,	// the material at position
	glm::vec3 const& P,			// world space position
	glm::vec3 const& N,			// normal at the position (already normalized)
	glm::vec3 const& V);		// view vector (already normalized)

/*
 *  Loops over all lights and evaluates the phong shading model with attenuation
 *
 *  Parameters:
 *		- context for accessing the lights and rendering parameters.
 *		- render_data specific for one pixel
 *		- material at the location to be shaded.
 *		- P the point to be shaded.
 *		- N the surface normal at P.
 *		- V the normalized view vector (from P to the camera)
 */
glm::vec3 evaluate_phong(
	RenderData &data,			// class containing raytracing information
	MaterialSample const& mat,	// the material at position
	glm::vec3 const& P,			// world space position
	glm::vec3 const& N,			// normal at the position (already normalized)
	glm::vec3 const& V);		// view vector (already normalized)

glm::vec3 evaluate_reflection(
	RenderData &data,			// class containing raytracing information
	int depth,					// the current recursion depth
	glm::vec3 const& P,			// world space position
	glm::vec3 const& N,			// normal at the position (already normalized)
	glm::vec3 const& V);		// view vector (already normalized)

glm::vec3 evaluate_transmission(
	RenderData &data,			// class containing raytracing information
	int depth,					// the current recursion depth
	glm::vec3 const& P,			// world space position
	glm::vec3 const& N,			// normal at the position (already normalized)
	glm::vec3 const& V,			// view vector (already normalized)
	float eta);					// the relative refraction index

glm::vec3 handle_transmissive_material_single_ior(
	RenderData &data,			// class containing raytracing information
	int depth,					// the current recursion depth
	glm::vec3 const& P,			// world space position
	glm::vec3 const& N,			// normal at the position (already normalized)
	glm::vec3 const& V,			// view vector (already normalized)
	float eta);					// the relative refraction index

glm::vec3 handle_transmissive_material(
	RenderData &data,					// class containing raytracing information
	int depth,							// the current recursion depth
	glm::vec3 const& P,					// world space position
	glm::vec3 const& N,					// normal at the position (already normalized)
	glm::vec3 const& V,					// view vector (already normalized)
	glm::vec3 const& eta_of_channel);	// relative refraction index of red, green and blue color channel

/*
 * Call this function to start or continue one path segment during recursive raytracing
 */
glm::vec3 trace_recursive(
	RenderData & data,
	Ray const& ray,
	int depth);

