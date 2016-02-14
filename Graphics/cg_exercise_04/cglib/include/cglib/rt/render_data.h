#pragma once

#include <cglib/rt/intersection.h>
#include <cglib/core/camera.h>

struct ThreadLocalData;
struct RaytracingContext;

/*
 * Rendering data that will be passed to the raytracer for each pixel
 */
struct RenderData
{
	RenderData(
		RaytracingContext const& context_,
		ThreadLocalData* tld_) :
		context(context_),
		tld(tld_)
	{}

	RaytracingContext const& context; 
	ThreadLocalData* tld;
	Intersection isect;
	int num_cast_rays = 0;
	float x = 0.0f;	// x-Coordinate of (Sub-)Pixel
	float y = 0.0f;	// y-Coordinate of (Sub-)Pixel
	Camera::Mode camera_mode = Camera::Mono;
};
