#pragma once

#include <cglib/rt/raytracing_parameters.h>
#include <cglib/core/assert.h>

#include <memory>

class Scene;

struct RaytracingContext
{
	RaytracingContext();
	~RaytracingContext();
	static RaytracingContext *get_active();

    RaytracingParameters params;
    std::shared_ptr<Scene> scene;
	std::unordered_map<std::string, std::shared_ptr<Scene>> scenes;

private:
	static RaytracingContext *current_context;
	static RaytracingContext *old_context;
};
