#include <cglib/core/assert.h>
#include <cglib/core/gui.h>
#include <cglib/core/image.h>
#include <cglib/core/timer.h>
#include <cglib/gl/device_render.h>
#include <cglib/gl/device_rendering_context.h>
#include <cglib/gl/renderer.h>
#include <cstring>

#ifdef _WIN32
	// Use discrete NVIDIA GPU on laptops
	extern "C" _declspec(dllexport) DWORD const NvOptimusEnablement = 1;
#endif

#define CREATE_DEBUG_CONTEXT 0

#if CREATE_DEBUG_CONTEXT == 1
#include <iostream>

static void APIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
{
	bool Error = false;
	bool Info = false;

	char const* srcS = "Unknown Source";
	if (source == GL_DEBUG_SOURCE_API_ARB)
		srcS = "OpenGL";
	else if (source == GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB)
		srcS = "Windows";
	else if (source == GL_DEBUG_SOURCE_SHADER_COMPILER_ARB)
		srcS = "Shader Compiler";
	else if (source == GL_DEBUG_SOURCE_THIRD_PARTY_ARB)
		srcS = "Third Party";
	else if (source == GL_DEBUG_SOURCE_APPLICATION_ARB)
		srcS = "Application";
	else if (source == GL_DEBUG_SOURCE_OTHER_ARB)
		srcS = "Other";

	char const* typeS = "unknown error";
	if (type == GL_DEBUG_TYPE_ERROR_ARB)
		typeS = "error";
	else if (type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB)
		typeS = "deprecated behavior";
	else if (type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB)
		typeS = "undefined behavior";
	else if (type == GL_DEBUG_TYPE_PORTABILITY_ARB)
		typeS = "portability";
	else if (type == GL_DEBUG_TYPE_PERFORMANCE_ARB)
		typeS = "performance";
	else if (type == GL_DEBUG_TYPE_OTHER_ARB)
	{
		typeS = "message";
		Info = false
			|| id == 131076 // NVIDIA: Small pointer value, intended to be used as an offset into a buffer? Yes.
			|| (strstr(message, "info:") != nullptr);
	}

	char const* severityS = "unknown severity";
	if (severity == GL_DEBUG_SEVERITY_HIGH_ARB)
	{
		severityS = "high";
		Error = true;
	}
	else if (severity == GL_DEBUG_SEVERITY_MEDIUM_ARB)
		severityS = "medium";
	else if (severity == GL_DEBUG_SEVERITY_LOW_ARB)
		severityS = "low";

	if (Error || !Info)
		std::cout << srcS << ": " << typeS << "(" << severityS << ") " << id << ": " << message << std::endl;
}
#endif

int DeviceRender::
run(DeviceRenderingContext &context, bool modern_gl)
{
	if(!GUI::init_device(context.params, modern_gl, bool(CREATE_DEBUG_CONTEXT))) {
		return 1;
	}

#if CREATE_DEBUG_CONTEXT == 1
	glEnable(GL_DEBUG_OUTPUT);
	glGetError();
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glGetError();
//	if (glDebugMessageControl)
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
//	if (glDebugMessageCallback)
		glDebugMessageCallback(&debugCallback, NULL);
#endif

	int status = 0;
	try {
		for(auto &i: context.renderers) {
			if(i.second->initialize(context.params)) {
				std::cerr << "error initializing renderers" << std::endl;
				return 1;
			}
		}
		cg_assert(context.get_current_renderer());
		context.get_current_renderer()->activate();
		context.get_current_renderer()->set_active_camera();

		if(context.params.interactive) {
			status = run_interactive(context, modern_gl);
		}
		else {
			status = run_noninteractive(context, modern_gl);
		}
		for(auto &i: context.renderers) {
			i.second->destroy();
		}
	}
	catch(...) {
		GUI::cleanup();
		throw;
	}
	GUI::cleanup();

	return status;
}

// -----------------------------------------------------------------------------

int DeviceRender::
run_noninteractive(DeviceRenderingContext &context, bool modern_gl)
{
	int status = 0;

	context.get_current_renderer()->update_objects(1e-8f);
	context.get_current_renderer()->draw();
	Image frame_buffer(context.params.image_width, context.params.image_height);
	glReadPixels(0, 0, context.params.image_width, context.params.image_height,
			GL_RGBA, GL_FLOAT, (float *)frame_buffer.getPixels());

	frame_buffer.saveTGA(context.params.output_file_name.c_str(), 1.0f);

	return status;
}

// -----------------------------------------------------------------------------

int DeviceRender::
run_interactive(DeviceRenderingContext &context, bool modern_gl)
{
	int status = 0;

	auto old_params = context.params;
	auto current_renderer = context.get_current_renderer();
	Timer timer;
	float frame_time = 1e-8f;
	while (GUI::keep_running())
	{
		timer.start();
		if (context.params.change_requires_restart(old_params))
		{
			old_params = context.params;
			for(auto &i: context.renderers) {
				if(i.second->update_parameters(context.params)) {
					status = 1;
					break;
				}
			}
			if(current_renderer != context.get_current_renderer()) {
				current_renderer->deactivate();
				current_renderer = context.get_current_renderer();
				current_renderer->activate();
				current_renderer->set_active_camera();
				context.params.gui_setup();
			}
		}

		GUI::poll_events();

		current_renderer->update_objects(frame_time);
		current_renderer->draw();
		GUI::display_device();
		timer.stop();
		frame_time = static_cast<float>(timer.getElapsedTimeInSec());
		timer.reset();
	}
	return status;
}


