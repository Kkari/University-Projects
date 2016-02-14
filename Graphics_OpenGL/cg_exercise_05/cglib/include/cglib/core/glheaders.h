#ifndef _GLHEADERS_H_
#define _GLHEADERS_H_

#define GLEW_STATIC

#include <GL/glew.h>
#if defined(_WIN32)
#  include <GL/wglew.h>
#elif !defined(__APPLE__) || defined(GLEW_APPLE_GLX)
#  include <GL/glxew.h>
#endif

#include <GLFW/glfw3.h>

#define GLERROR do { \
    GLint err;                          \
    while ((err = glGetError()) != GL_NO_ERROR) {                           \
        /*__debugbreak();*/ \
        GLubyte const* str = gluErrorString(err);       \
        std::cout << __FILE__ << ":" << __LINE__ << " : GLERROR: " << str << std::endl; \
    }                                                   \
} while(0)

#endif //_GLHEADERS_H_
