#ifndef  __SHADER_HPP__
#define  __SHADER_HPP__

#include <cglib/core/glheaders.h>

#include <string>
#include <iostream>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <cglib/gl/opengl_types.h>

struct ProgramDefinition
{
	/* specifies which files to load for a shader program
	 * furthermore defines which varyings are used for
	 * transform feedback */
	std::vector<const char *> files_vertex,
	                          files_fragment,
	                          feedback_varyings,
	                          files_geom;
};

typedef std::unordered_map<std::string, ProgramDefinition> MapNameProgramDefinition;

struct ShaderSource;

class Shader
{
public:
	unsigned int program;

	Shader() : program(0) {}
	Shader(const ShaderSource &shader_src, const ShaderSource *src_orig = nullptr);

	~Shader()
	{
		glDeleteProgram(program);
	}

	Shader &
	bind()
	{
		glUseProgram(program);
		return *this;
	}
	
	template<typename T>
	Shader &
	uniform(const std::string &str, const T &t)
	{
		return uniform(str, &t, 1);
	}

	template<typename T>
	Shader &
	uniform(const std::string &str, const T *t, size_t count)
	{
		auto it = uniforms.find(str);
		if(it != uniforms.end()) {
			Uniform &u = it->second;
			if(!u.check_type(*t))
				u.assign(t, count);
			else
				std::cerr << "Error: type mismatch for uniform variable \"" << str << "\"!" << std::endl;
		}
		else {
			if(reported_missing_uniforms.find(str) == reported_missing_uniforms.end()) {
				std::cerr << "Warning: uniform \"" << str << "\" not found in shader!" << std::endl;
				reported_missing_uniforms.insert(str);
			}
		}

		return *this;
	}

private:
	struct Uniform
	{
		int loc, type;
		Uniform() {}
		Uniform(int l, int t) : loc(l), type(t) {}
		void assign(const float *f, size_t n)      { glUniform1fv(loc, (GLsizei) n, f); }
		void assign(const int *i, size_t n)        { glUniform1iv(loc, (GLsizei) n, i); }
		void assign(const unsigned *i, size_t n)   { glUniform1uiv(loc, (GLsizei) n, i); }
		void assign(const glm::ivec2 *v, size_t n) { glUniform2iv(loc, (GLsizei) n, (const int*)v); }
		void assign(const glm::ivec3 *v, size_t n) { glUniform3iv(loc, (GLsizei) n, (const int*)v); }
		void assign(const glm::vec2 *v, size_t n)  { glUniform2fv(loc, (GLsizei) n, (const float*)v); }
		void assign(const glm::vec3 *v, size_t n)  { glUniform3fv(loc, (GLsizei) n, (const float*)v); }
		void assign(const glm::vec4 *v, size_t n)  { glUniform4fv(loc, (GLsizei) n, (const float*)v); }
		void assign(const glm::mat2 *m, size_t n)  { glUniformMatrix2fv(loc, (GLsizei) n, 0, (const float*)m); }
		void assign(const glm::mat3 *m, size_t n)  { glUniformMatrix3fv(loc, (GLsizei) n, 0, (const float*)m); }
		void assign(const glm::mat4 *m, size_t n)  { glUniformMatrix4fv(loc, (GLsizei) n, 0, (const float*)m); }

		template<typename T>
		int
		check_type(const T &t)
		{
			return type != TYPE_TO_GLTYPE(T);
		}

		int
		check_type(const int &t)
		{
			return t == TYPE_TO_GLTYPE(int)
				|| t == GL_SAMPLER_1D
				|| t == GL_SAMPLER_2D
				|| t == GL_SAMPLER_3D
				|| t == GL_SAMPLER_CUBE
				|| t == GL_SAMPLER_1D_SHADOW
				|| t == GL_SAMPLER_2D_SHADOW
				|| t == GL_SAMPLER_1D_ARRAY
				|| t == GL_SAMPLER_2D_ARRAY
				|| t == GL_SAMPLER_1D_ARRAY_SHADOW
				|| t == GL_SAMPLER_2D_ARRAY_SHADOW
				|| t == GL_SAMPLER_CUBE_SHADOW
				|| t == GL_SAMPLER_2D_RECT
				|| t == GL_SAMPLER_2D_RECT_SHADOW
				|| t == GL_SAMPLER_2D_MULTISAMPLE
				|| t == GL_SAMPLER_2D_MULTISAMPLE_ARRAY
				|| t == GL_SAMPLER_CUBE_MAP_ARRAY
				|| t == GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW;
		}
	};

	std::unordered_map<std::string, Uniform> uniforms;
	std::unordered_set<std::string> reported_missing_uniforms;
};

class ShaderManager
{
public:
	ShaderManager(const MapNameProgramDefinition &program_definitions_)
		: program_definitions(program_definitions_)
	{}

	void reload_shader();

	Shader &
#ifdef __GNUC__
		__attribute__((warn_unused_result)) 
#endif
		operator[](const std::string &str);
	int load_shader(const std::string &path);
private:
	std::unordered_map<std::string, Shader *> shaders;
	const MapNameProgramDefinition &program_definitions;
};

#endif  /*__SHADER_HPP__*/
