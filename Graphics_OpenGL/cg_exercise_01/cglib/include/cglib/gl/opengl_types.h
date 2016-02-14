#ifndef  __OPENGL_TYPES_HPP__
#define  __OPENGL_TYPES_HPP__


#include <GL/glew.h>
#include <glm/glm.hpp>

namespace GLUtil {

template<typename T> struct type_to_gltype { enum { type = 0 }; };
template<int x> struct gltype_to_type { typedef void type; };

#define _OGL_TYPE_MAPPING(_type, _ogltype) \
    template<> struct type_to_gltype< _type > { enum { type = _ogltype }; };\
    template<> struct gltype_to_type< _ogltype > { typedef _type type; }

_OGL_TYPE_MAPPING(float,        GL_FLOAT);
_OGL_TYPE_MAPPING(double,       GL_DOUBLE);
_OGL_TYPE_MAPPING(char,         GL_UNSIGNED_BYTE);
_OGL_TYPE_MAPPING(unsigned int, GL_UNSIGNED_INT);
_OGL_TYPE_MAPPING(int,          GL_INT);
_OGL_TYPE_MAPPING(glm::ivec2,   GL_INT_VEC2);
_OGL_TYPE_MAPPING(glm::ivec3,   GL_INT_VEC3);
_OGL_TYPE_MAPPING(glm::vec2,    GL_FLOAT_VEC2);
_OGL_TYPE_MAPPING(glm::vec3,    GL_FLOAT_VEC3);
_OGL_TYPE_MAPPING(glm::vec4,    GL_FLOAT_VEC4);
_OGL_TYPE_MAPPING(glm::mat2,    GL_FLOAT_MAT2);
_OGL_TYPE_MAPPING(glm::mat3,    GL_FLOAT_MAT3);
_OGL_TYPE_MAPPING(glm::mat4,    GL_FLOAT_MAT4);


#undef _OGL_TYPE_MAPPING

#define TYPE_TO_GLTYPE(a) (GLUtil::type_to_gltype<a>::type)
#define GLTYPE_TO_TYPE(a) (GLUtil::gltype_to_type<a>::type)

}


#endif  /*__OPENGL_TYPES_HPP__*/
