#pragma once

#include <memory>
#include <unordered_map>
#include <string>

#include <cglib/core/assert.h>
#include <cglib/gl/device_rendering_parameters.h>

class Renderer;

class DeviceRenderingContext
{
public:
	DeviceRenderingContext();
	~DeviceRenderingContext();
	std::shared_ptr<Renderer> get_current_renderer() const;
	static DeviceRenderingContext *get_active();
	static void reload_all_shaders();

    DeviceRenderingParameters params;
	std::unordered_map<
		DeviceRenderingParameters::Renderers,
		std::shared_ptr<Renderer>,
		std::hash<int>
	> renderers;

private:
	static DeviceRenderingContext *current_context;
};
