#pragma once

#include <functional>
#include <cglib/core/gui.h>
#include <cglib/gl/device_rendering_parameters.h>

class DeviceRenderingContext;

/*
 * Use this class to render on the device (so primarily with OpenGL).
 */
class DeviceRender
{
	public:
		/*
		 * The render and initialization functions.
		 */
		typedef std::function<bool(DeviceRenderingContext const&)> InitFunc;
		typedef std::function<void(DeviceRenderingContext const&)> RenderFunc;

		static int run(DeviceRenderingContext & context, bool modern_gl=true);

		/*
		 * creates a context, executes func() and destroys the context
		 */
		static inline int run_in_context(
				DeviceRenderingParameters &params,
				bool modern_gl,
				std::function<void()> func);

	private:
		static int run_interactive(DeviceRenderingContext & context, bool modern_gl);
		static int run_noninteractive(DeviceRenderingContext & context, bool modern_gl);
};

inline int DeviceRender::
run_in_context(DeviceRenderingParameters &params, bool modern_gl, std::function<void()> func)
{
	if(!GUI::init_device(params, modern_gl))
		return 1;
	try {
		func();
	} catch(...) {
		GUI::cleanup();
		throw;
	}
	GUI::cleanup();
	return 0;
}
