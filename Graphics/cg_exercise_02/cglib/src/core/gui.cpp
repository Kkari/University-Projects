#include <cglib/core/parameters.h>
#include <cglib/core/glheaders.h>
#include <cglib/core/camera.h>
#include <cglib/core/image.h>
#include <cglib/core/gui.h>

#include <AntTweakBar.h>

#include <iostream>
#include <algorithm>
#include <functional>
#include <cstdlib>

using std::cout;
using std::cerr;
using std::endl;
using std::flush;

static GLFWwindow *window;
static unsigned int tex_frame_buffer;
static unsigned int program_draw_texture;
static double cursor_x, cursor_y;
static bool in_camera_drag = false;

// this is for retina displays where
// we need to pass modified coordinates
// to antweakbar
static float scale_retina_x = 1.f;
static float scale_retina_y = 1.f;

static bool write_screenshot = false;

static Parameters* parameters = nullptr;

// -----------------------------------------------------------------------------

static void
glfw_error_callback(int error, const char *description)
{
	cerr << "GLFW Error: " << description << endl;
}

static int GLFW3ToTwKeyEvent(int key, int action);
static void GLFW3ToTwCharEvent(unsigned int chr);

static void
glfw_key_callback(GLFWwindow *win, int key, int scancode, int action, int mods)
{
	if(GLFW3ToTwKeyEvent(key, action)) {
		return;
	}

	auto cam = Camera::get_active();
	if (cam && cam->handle_key_event(key, action)) {
		return;
	}

	switch(key) {
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(win, GL_TRUE);
		return;
	case GLFW_KEY_F4:
		write_screenshot |= (action == GLFW_PRESS);
		break;
	default:
		break;
	}
}

static void
glfw_char_callback(GLFWwindow *win, unsigned int chr)
{
	GLFW3ToTwCharEvent(chr);
}

static void
glfw_button_callback(GLFWwindow *win, int button, int action, int mods)
{
	if (!in_camera_drag)
	{
		// normal event cascade that takes priority over cam
		if(TwEventMouseButtonGLFW(button, action)) {
			return;
		}
	}
	// camera drag end
	else if (button == 0 && action == GLFW_RELEASE)
	{
		glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		in_camera_drag = false;
	}

	auto cam = Camera::get_active();
	if (cam && cam->handle_mouse_button_event(button, action)) {
		return;
	}
}

static void
glfw_motion_callback(GLFWwindow *win, double x, double y)
{
	if (!in_camera_drag)
	{
		// normal event cascade that takes priority over cam
		if (TwEventMousePosGLFW(static_cast<int>(scale_retina_x * x), static_cast<int>(scale_retina_y * y))) {
			return;
		}
	}

	auto cam = Camera::get_active();
	if (glfwGetMouseButton(win, 0) == GLFW_PRESS
		&& cam && cam->handle_mouse_drag_event(static_cast<float>(cursor_x - x), static_cast<float>(cursor_y - y)))
	{
		if (!in_camera_drag)
		{
			// drag start
//			glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // broken on too many platforms right now, retry with newer version of GLFW
			in_camera_drag = true;
		}

		cursor_x = x;
		cursor_y = y;
		return;
	}

    cursor_x = x;
    cursor_y = y;
}

static void
glfw_mouse_wheel_callback(GLFWwindow *win, double x, double y)
{
	if(TwEventMouseWheelGLFW(int(y))) {
		return;
	}
	auto cam = Camera::get_active();
	if (cam && cam->handle_mouse_wheel_event(static_cast<float>(x), static_cast<float>(y)))
	{
		return;
	}
}

static void
glfw_cursor_enter_callback(GLFWwindow *win, int i)
{
    glfwGetCursorPos(win, &cursor_x, &cursor_y);
}

static void
glfw_resize_callback(GLFWwindow *win, int w, int h)
{
#ifndef __APPLE__
	glViewport(0, 0, w, h);
	TwWindowSize(w, h);

	if (parameters)
	{
		parameters->screen_width = w;
		parameters->screen_height = h;
	}
#endif
}

#ifdef __APPLE__
static void
glfw_framebuffer_resize_callback(GLFWwindow *win, int w, int h)
{
	glViewport(0, 0, w, h);
	TwWindowSize(w, h);

	if (parameters)
	{
		parameters->screen_width = w;
		parameters->screen_height = h;
	}
}
#endif

// -----------------------------------------------------------------------------

static unsigned int
compile_src(const std::vector<const char *> &src, int type)
{
	GLuint shader_obj = glCreateShader(type);

	// glShaderSource wants char const**, not char const* const*.
	std::vector<const char*> srcs(src);

	glShaderSource(shader_obj, src.size(), srcs.data(), nullptr);
	glCompileShader(shader_obj);

	int success;
	glGetShaderiv(shader_obj, GL_COMPILE_STATUS, &success);
	if(!success) {
		char infolog[2048];
		glGetShaderInfoLog(shader_obj, sizeof infolog, NULL, infolog);
		printf("error compiling shader: %s\n", infolog);
	}

	return shader_obj;
}

static unsigned int
compile_shader(
		const std::vector<const char *> &src_vertex,
		const std::vector<const char *> &src_fragment)
{
	GLuint prog = glCreateProgram();

	unsigned int vert = 0, frag = 0;

	glAttachShader(prog, vert = compile_src(src_vertex, GL_VERTEX_SHADER));
	glAttachShader(prog, frag = compile_src(src_fragment, GL_FRAGMENT_SHADER));
	glLinkProgram(prog);

	int success;
	glGetProgramiv(prog, GL_LINK_STATUS, &success);
	if(!success) {
		char infolog[2048];
		glGetProgramInfoLog(prog, sizeof infolog, NULL, infolog);
		printf("error linking shaderprogram: %s\n", infolog);
	}

	glDeleteShader(vert);
	glDeleteShader(frag);

	return prog;
}

// -----------------------------------------------------------------------------

bool
create_context(Parameters const& params, bool modern_gl, bool debug_context)
{
	glfwSetErrorCallback(glfw_error_callback);
	if(!glfwInit())
		return false;

	if (debug_context)
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

	if(!params.interactive) /* window should have fixed size for testing */
		glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	if(modern_gl) {
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);
	#ifdef __linux__
		/* check if using mesa, context flags don't work with mesa */
		if(!getenv("MESA_GL_VERSION_OVERRIDE"))
	#endif
		{
			glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		}
	}
	else {
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
	}
	/* hiding the window will discard all pixel operations on the window
	 * frame buffer. To obtain the results, it is required to render into
	 * a FBO */
	glfwWindowHint(GLFW_VISIBLE, params.interactive);

	if(!(window = glfwCreateWindow(params.screen_width, params.screen_height, "cg framework", nullptr, nullptr))) {
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, glfw_key_callback);
	glfwSetCharCallback(window, glfw_char_callback);
	glfwSetCursorPosCallback(window, glfw_motion_callback);
	glfwSetMouseButtonCallback(window, glfw_button_callback);
	glfwSetScrollCallback(window, glfw_mouse_wheel_callback);
	glfwSetWindowSizeCallback(window, glfw_resize_callback);
#ifdef __APPLE__
	glfwSetFramebufferSizeCallback(window, glfw_framebuffer_resize_callback);
#endif
	glfwSetCursorEnterCallback(window, glfw_cursor_enter_callback);

	if (modern_gl)
	{
		glewExperimental = GL_TRUE;
	}
	GLenum err = glewInit();
	if(err != GLEW_OK) {
		cerr << "Error: " << glewGetErrorString(err) << endl;
		return false;
	}
	while(glGetError() != GL_NO_ERROR);

#ifdef __APPLE__
	{
		int win_width, win_height;
		glfwGetWindowSize(window, &win_width, &win_height);
		int fb_width, fb_height;
		glfwGetFramebufferSize(window, &fb_width, &fb_height);
		scale_retina_x = float(fb_width)  / float(win_width);
		scale_retina_y = float(fb_height) / float(win_height);
		std::cout
			<< "scale for retina? scale_x: " << scale_retina_x
			<< ", scale_y: " << scale_retina_y << std::endl;
	}
#endif

	return true;
}

void
init_host_texture(Parameters const& params)
{
	program_draw_texture = compile_shader({
"#version 120\n"
"varying vec2 tex_coord;\n"
"uniform float w;\n"
"uniform float h;\n"
"void\n"
"main()\n"
"{\n"
"	vec2 v;\n"
"	vec4 p = gl_Vertex;\n"
"	tex_coord = p.xy * 0.5 + 0.5;\n"
"	gl_Position = vec4(vec2(w, h) * p.xy, 0.0, 1.0);\n"
"}\n"} , {
"#version 120\n"
"varying vec2 tex_coord;\n"
"uniform float gamma;\n"
"uniform float exposure;\n"
"uniform sampler2D framebuffer;\n"
"void\n"
"main()\n"
"{\n"
"	gl_FragColor = pow(exposure * texture2D(framebuffer, tex_coord), vec4(gamma));\n"
"}\n" }
);
	glGenTextures(1, &tex_frame_buffer);
	glBindTexture(GL_TEXTURE_2D, tex_frame_buffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, params.image_width, params.image_height, 0,
		GL_RGBA, GL_FLOAT, nullptr);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

// -----------------------------------------------------------------------------

static bool
init_tweakbar(const Parameters &params, bool modern_gl)
{
	int w = params.screen_width;
	int h = params.screen_height;
#ifdef __APPLE__
	int fb_width, fb_height;
	glfwGetFramebufferSize(window, &w, &h);
#endif

	if(params.interactive) {
		if(!TwInit(modern_gl ? TW_OPENGL_CORE : TW_OPENGL, nullptr)) {
			std::cerr
				<< "AntTweakBar initialization failed: %s\n" << TwGetLastError()
				<< std::endl;
			return false;
		}
		TwWindowSize(w, h);
	}

	return true;
}

bool GUI::
keep_running()
{
	return !glfwWindowShouldClose(window);
}

void GUI::
cleanup()
{
	glfwDestroyWindow(window);
	TwTerminate();
	glfwTerminate();
}

void GUI::
poll_events()
{
	return glfwPollEvents();
}

void GUI::
set_parameters_ptr(Parameters* p)
{
	parameters = p;
}

bool GUI::
init_host(Parameters& params)
{
	GUI::set_parameters_ptr(&params);

	if(!create_context(params, false, false)) {
		return false;
	}

	if(!init_tweakbar(params, false)) {
		return false;
	}

	glClearColor(0, 0, 0, 0);
	init_host_texture(params);
	params.gui_setup();
	return true;
}

void GUI::
display_host(Image const& frame_buffer, std::function<void()> const& render_overlay)
{
	if (write_screenshot)
	{
		write_screenshot = false;
		frame_buffer.saveTGA("screenshot.tga", 2.2f);
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float w = float(parameters->screen_height) / float(frame_buffer.getHeight());
	float h = float(parameters->screen_width)  / float(frame_buffer.getWidth());
	float tmp = std::max < float> (w, h);
	{
		w /= tmp;
		h /= tmp;
	}

	glUseProgram(program_draw_texture);
	glUniform1f(glGetUniformLocation(program_draw_texture, "w"), w);
	glUniform1f(glGetUniformLocation(program_draw_texture, "h"), h);
	glUniform1f(glGetUniformLocation(program_draw_texture, "gamma"), 1.0f / parameters->gamma);
	glUniform1f(glGetUniformLocation(program_draw_texture, "exposure"), parameters->exposure);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_frame_buffer);
	glTexSubImage2D(GL_TEXTURE_2D, 0, /* x offset */ 0, /* y offset */ 0,
		frame_buffer.getWidth(), frame_buffer.getHeight(), GL_RGBA, GL_FLOAT, frame_buffer.raw_data());
	glGenerateMipmap(GL_TEXTURE_2D);
	glRectf(-1.0f, -1.0f, 1.0f, 1.0f);

	render_overlay();
	TwDraw();
	glfwSwapBuffers(window);
}

bool GUI::
init_device(Parameters& params, bool modern_gl, bool debug_context)
{
	GUI::set_parameters_ptr(&params);

	if(!create_context(params, modern_gl, debug_context)) {
		return false;
	}
	if(!init_tweakbar(params, modern_gl)) {
		return false;
	}

	params.gui_setup();
	return true;
}

void GUI::
display_device()
{
	glUseProgram(0); // avoid shaders leaking into tweak bar

	if (write_screenshot)
	{
		write_screenshot = false;
		Image frame_buffer(parameters->screen_width, parameters->screen_height);
		glReadPixels(0, 0, frame_buffer.getWidth(), frame_buffer.getHeight(), GL_RGBA, GL_FLOAT, frame_buffer.getPixels());
		frame_buffer.saveTGA("screenshot.tga", 1.0f);
	}

	TwDraw();
	glfwSwapBuffers(window);
}

static void GLFW3ToTwCharEvent(unsigned int chr)
{
	TwEventCharGLFW(chr, GLFW_PRESS);
}

static int GLFW3ToTwKeyEvent(int key, int action)
{
	#define GLFW2_KEY_UNKNOWN      -1
	#define GLFW2_KEY_SPACE        32
	#define GLFW2_KEY_SPECIAL      256
	#define GLFW2_KEY_ESC          (GLFW2_KEY_SPECIAL+1)
	#define GLFW2_KEY_F1           (GLFW2_KEY_SPECIAL+2)
	#define GLFW2_KEY_F2           (GLFW2_KEY_SPECIAL+3)
	#define GLFW2_KEY_F3           (GLFW2_KEY_SPECIAL+4)
	#define GLFW2_KEY_F4           (GLFW2_KEY_SPECIAL+5)
	#define GLFW2_KEY_F5           (GLFW2_KEY_SPECIAL+6)
	#define GLFW2_KEY_F6           (GLFW2_KEY_SPECIAL+7)
	#define GLFW2_KEY_F7           (GLFW2_KEY_SPECIAL+8)
	#define GLFW2_KEY_F8           (GLFW2_KEY_SPECIAL+9)
	#define GLFW2_KEY_F9           (GLFW2_KEY_SPECIAL+10)
	#define GLFW2_KEY_F10          (GLFW2_KEY_SPECIAL+11)
	#define GLFW2_KEY_F11          (GLFW2_KEY_SPECIAL+12)
	#define GLFW2_KEY_F12          (GLFW2_KEY_SPECIAL+13)
	#define GLFW2_KEY_F13          (GLFW2_KEY_SPECIAL+14)
	#define GLFW2_KEY_F14          (GLFW2_KEY_SPECIAL+15)
	#define GLFW2_KEY_F15          (GLFW2_KEY_SPECIAL+16)
	#define GLFW2_KEY_F16          (GLFW2_KEY_SPECIAL+17)
	#define GLFW2_KEY_F17          (GLFW2_KEY_SPECIAL+18)
	#define GLFW2_KEY_F18          (GLFW2_KEY_SPECIAL+19)
	#define GLFW2_KEY_F19          (GLFW2_KEY_SPECIAL+20)
	#define GLFW2_KEY_F20          (GLFW2_KEY_SPECIAL+21)
	#define GLFW2_KEY_F21          (GLFW2_KEY_SPECIAL+22)
	#define GLFW2_KEY_F22          (GLFW2_KEY_SPECIAL+23)
	#define GLFW2_KEY_F23          (GLFW2_KEY_SPECIAL+24)
	#define GLFW2_KEY_F24          (GLFW2_KEY_SPECIAL+25)
	#define GLFW2_KEY_F25          (GLFW2_KEY_SPECIAL+26)
	#define GLFW2_KEY_UP           (GLFW2_KEY_SPECIAL+27)
	#define GLFW2_KEY_DOWN         (GLFW2_KEY_SPECIAL+28)
	#define GLFW2_KEY_LEFT         (GLFW2_KEY_SPECIAL+29)
	#define GLFW2_KEY_RIGHT        (GLFW2_KEY_SPECIAL+30)
	#define GLFW2_KEY_LSHIFT       (GLFW2_KEY_SPECIAL+31)
	#define GLFW2_KEY_RSHIFT       (GLFW2_KEY_SPECIAL+32)
	#define GLFW2_KEY_LCTRL        (GLFW2_KEY_SPECIAL+33)
	#define GLFW2_KEY_RCTRL        (GLFW2_KEY_SPECIAL+34)
	#define GLFW2_KEY_LALT         (GLFW2_KEY_SPECIAL+35)
	#define GLFW2_KEY_RALT         (GLFW2_KEY_SPECIAL+36)
	#define GLFW2_KEY_TAB          (GLFW2_KEY_SPECIAL+37)
	#define GLFW2_KEY_ENTER        (GLFW2_KEY_SPECIAL+38)
	#define GLFW2_KEY_BACKSPACE    (GLFW2_KEY_SPECIAL+39)
	#define GLFW2_KEY_INSERT       (GLFW2_KEY_SPECIAL+40)
	#define GLFW2_KEY_DEL          (GLFW2_KEY_SPECIAL+41)
	#define GLFW2_KEY_PAGEUP       (GLFW2_KEY_SPECIAL+42)
	#define GLFW2_KEY_PAGEDOWN     (GLFW2_KEY_SPECIAL+43)
	#define GLFW2_KEY_HOME         (GLFW2_KEY_SPECIAL+44)
	#define GLFW2_KEY_END          (GLFW2_KEY_SPECIAL+45)
	#define GLFW2_KEY_KP_0         (GLFW2_KEY_SPECIAL+46)
	#define GLFW2_KEY_KP_1         (GLFW2_KEY_SPECIAL+47)
	#define GLFW2_KEY_KP_2         (GLFW2_KEY_SPECIAL+48)
	#define GLFW2_KEY_KP_3         (GLFW2_KEY_SPECIAL+49)
	#define GLFW2_KEY_KP_4         (GLFW2_KEY_SPECIAL+50)
	#define GLFW2_KEY_KP_5         (GLFW2_KEY_SPECIAL+51)
	#define GLFW2_KEY_KP_6         (GLFW2_KEY_SPECIAL+52)
	#define GLFW2_KEY_KP_7         (GLFW2_KEY_SPECIAL+53)
	#define GLFW2_KEY_KP_8         (GLFW2_KEY_SPECIAL+54)
	#define GLFW2_KEY_KP_9         (GLFW2_KEY_SPECIAL+55)
	#define GLFW2_KEY_KP_DIVIDE    (GLFW2_KEY_SPECIAL+56)
	#define GLFW2_KEY_KP_MULTIPLY  (GLFW2_KEY_SPECIAL+57)
	#define GLFW2_KEY_KP_SUBTRACT  (GLFW2_KEY_SPECIAL+58)
	#define GLFW2_KEY_KP_ADD       (GLFW2_KEY_SPECIAL+59)
	#define GLFW2_KEY_KP_DECIMAL   (GLFW2_KEY_SPECIAL+60)
	#define GLFW2_KEY_KP_EQUAL     (GLFW2_KEY_SPECIAL+61)
	#define GLFW2_KEY_KP_ENTER     (GLFW2_KEY_SPECIAL+62)
	#define GLFW2_KEY_KP_NUM_LOCK  (GLFW2_KEY_SPECIAL+63)
	#define GLFW2_KEY_CAPS_LOCK    (GLFW2_KEY_SPECIAL+64)
	#define GLFW2_KEY_SCROLL_LOCK  (GLFW2_KEY_SPECIAL+65)
	#define GLFW2_KEY_PAUSE        (GLFW2_KEY_SPECIAL+66)
	#define GLFW2_KEY_LSUPER       (GLFW2_KEY_SPECIAL+67)
	#define GLFW2_KEY_RSUPER       (GLFW2_KEY_SPECIAL+68)
	#define GLFW2_KEY_MENU         (GLFW2_KEY_SPECIAL+69)
	#define GLFW2_KEY_LAST         GLFW2_KEY_MENU

	switch (key)
	{
	case GLFW_KEY_SPACE:			key = GLFW2_KEY_SPACE;			break;
	case GLFW_KEY_ESCAPE:			key = GLFW2_KEY_ESC;			break;
	case GLFW_KEY_F1:				key = GLFW2_KEY_F1;				break;
	case GLFW_KEY_F2:				key = GLFW2_KEY_F2;				break;
	case GLFW_KEY_F3:				key = GLFW2_KEY_F3;				break;
	case GLFW_KEY_F4:				key = GLFW2_KEY_F4;				break;
	case GLFW_KEY_F5:				key = GLFW2_KEY_F5;				break;
	case GLFW_KEY_F6:				key = GLFW2_KEY_F6;				break;
	case GLFW_KEY_F7:				key = GLFW2_KEY_F7;				break;
	case GLFW_KEY_F8:				key = GLFW2_KEY_F8;				break;
	case GLFW_KEY_F9:				key = GLFW2_KEY_F9;				break;
	case GLFW_KEY_F10:				key = GLFW2_KEY_F10;			break;
	case GLFW_KEY_F11:				key = GLFW2_KEY_F11;			break;
	case GLFW_KEY_F12:				key = GLFW2_KEY_F12;			break;
	case GLFW_KEY_F13:				key = GLFW2_KEY_F13;			break;
	case GLFW_KEY_F14:				key = GLFW2_KEY_F14;			break;
	case GLFW_KEY_F15:				key = GLFW2_KEY_F15;			break;
	case GLFW_KEY_F16:				key = GLFW2_KEY_F16;			break;
	case GLFW_KEY_F17:				key = GLFW2_KEY_F17;			break;
	case GLFW_KEY_F18:				key = GLFW2_KEY_F18;			break;
	case GLFW_KEY_F19:				key = GLFW2_KEY_F19;			break;
	case GLFW_KEY_F20:				key = GLFW2_KEY_F20;			break;
	case GLFW_KEY_F21:				key = GLFW2_KEY_F21;			break;
	case GLFW_KEY_F22:				key = GLFW2_KEY_F22;			break;
	case GLFW_KEY_F23:				key = GLFW2_KEY_F23;			break;
	case GLFW_KEY_F24:				key = GLFW2_KEY_F24;			break;
	case GLFW_KEY_F25:				key = GLFW2_KEY_F25;			break;
	case GLFW_KEY_UP:				key = GLFW2_KEY_UP;				break;
	case GLFW_KEY_DOWN:				key = GLFW2_KEY_DOWN;			break;
	case GLFW_KEY_LEFT:				key = GLFW2_KEY_LEFT;			break;
	case GLFW_KEY_RIGHT:			key = GLFW2_KEY_RIGHT;			break;
	case GLFW_KEY_LEFT_SHIFT:		key = GLFW2_KEY_LSHIFT;			break;
	case GLFW_KEY_RIGHT_SHIFT:		key = GLFW2_KEY_RSHIFT;			break;
	case GLFW_KEY_LEFT_CONTROL:		key = GLFW2_KEY_LCTRL;			break;
	case GLFW_KEY_RIGHT_CONTROL:	key = GLFW2_KEY_RCTRL;			break;
	case GLFW_KEY_LEFT_ALT:			key = GLFW2_KEY_LALT;			break;
	case GLFW_KEY_RIGHT_ALT:		key = GLFW2_KEY_RALT;			break;
	case GLFW_KEY_TAB:				key = GLFW2_KEY_TAB;			break;
	case GLFW_KEY_ENTER:			key = GLFW2_KEY_ENTER;			break;
	case GLFW_KEY_BACKSPACE:		key = GLFW2_KEY_BACKSPACE;		break;
	case GLFW_KEY_INSERT:			key = GLFW2_KEY_INSERT;			break;
	case GLFW_KEY_DELETE:			key = GLFW2_KEY_DEL;			break;
	case GLFW_KEY_PAGE_UP:			key = GLFW2_KEY_PAGEUP;			break;
	case GLFW_KEY_PAGE_DOWN:		key = GLFW2_KEY_PAGEDOWN;		break;
	case GLFW_KEY_HOME:				key = GLFW2_KEY_HOME;			break;
	case GLFW_KEY_END:				key = GLFW2_KEY_END;			break;
	case GLFW_KEY_KP_0:				key = GLFW2_KEY_KP_0;			break;
	case GLFW_KEY_KP_1:				key = GLFW2_KEY_KP_1;			break;
	case GLFW_KEY_KP_2:				key = GLFW2_KEY_KP_2;			break;
	case GLFW_KEY_KP_3:				key = GLFW2_KEY_KP_3;			break;
	case GLFW_KEY_KP_4:				key = GLFW2_KEY_KP_4;			break;
	case GLFW_KEY_KP_5:				key = GLFW2_KEY_KP_5;			break;
	case GLFW_KEY_KP_6:				key = GLFW2_KEY_KP_6;			break;
	case GLFW_KEY_KP_7:				key = GLFW2_KEY_KP_7;			break;
	case GLFW_KEY_KP_8:				key = GLFW2_KEY_KP_8;			break;
	case GLFW_KEY_KP_9:				key = GLFW2_KEY_KP_9;			break;
	case GLFW_KEY_KP_DIVIDE:		key = GLFW2_KEY_KP_DIVIDE;		break;
	case GLFW_KEY_KP_MULTIPLY:		key = GLFW2_KEY_KP_MULTIPLY;	break;
	case GLFW_KEY_KP_SUBTRACT:		key = GLFW2_KEY_KP_SUBTRACT;	break;
	case GLFW_KEY_KP_ADD:			key = GLFW2_KEY_KP_ADD;			break;
	case GLFW_KEY_KP_DECIMAL:		key = GLFW2_KEY_KP_DECIMAL;		break;
	case GLFW_KEY_KP_EQUAL:			key = GLFW2_KEY_KP_EQUAL;		break;
	case GLFW_KEY_KP_ENTER:			key = GLFW2_KEY_KP_ENTER;		break;
	case GLFW_KEY_NUM_LOCK:			key = GLFW2_KEY_KP_NUM_LOCK;	break;
	case GLFW_KEY_CAPS_LOCK:		key = GLFW2_KEY_CAPS_LOCK;		break;
	case GLFW_KEY_SCROLL_LOCK:		key = GLFW2_KEY_SCROLL_LOCK;	break;
	case GLFW_KEY_PAUSE:			key = GLFW2_KEY_PAUSE;			break;
	case GLFW_KEY_APOSTROPHE:		key = '\'';						break;
	case GLFW_KEY_COMMA:			key = ',';						break;
	case GLFW_KEY_MINUS:			key = '-';						break;
	case GLFW_KEY_PERIOD:			key = '.';						break;
	case GLFW_KEY_SLASH:			key = '/';						break;
	case GLFW_KEY_SEMICOLON:		key = ';';						break;
	case GLFW_KEY_EQUAL:			key = '=';						break;
	case GLFW_KEY_LEFT_BRACKET:		key = '[';						break;
	case GLFW_KEY_BACKSLASH:		key = '\\';						break;
	case GLFW_KEY_RIGHT_BRACKET:	key = ']';						break;
	case GLFW_KEY_GRAVE_ACCENT:		key = '`';						break;
	default:
		if (GLFW_KEY_A <= key && key <= GLFW_KEY_Z)
			key = key - GLFW_KEY_A + 'A';
		else if (GLFW_KEY_0 <= key && key <= GLFW_KEY_9)
			key = key - GLFW_KEY_0 + '0';
		else
			// cannot translate, skip char
			return 0;
	}

	// assert: key translated
	// pass on if not a repeat event
	return (action != GLFW_REPEAT) ? TwEventKeyGLFW(key, action) : 0;
}
