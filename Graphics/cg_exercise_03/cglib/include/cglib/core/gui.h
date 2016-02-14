#pragma once

#include <cglib/core/parameters.h>
#include <cglib/core/image.h>

#include <functional>

class Camera;

namespace GUI {
	bool keep_running();
	void cleanup();
	void poll_events();

	bool init_host(Parameters& params);
	void display_host(Image const& frame_buffer, std::function<void()> const& render_overlay);

	bool init_device(Parameters& params, bool modern_gl, bool debug_context = false);
	void display_device();

    void set_parameters_ptr(Parameters* parameters);
}

