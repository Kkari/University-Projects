#include <cglib/core/glheaders.h>
#include <cglib/gl/shader.h>
#include <cglib/core/file_util.h>
#include <cglib/core/assert.h>


#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <string.h>
#include <vector>

#define LENGTH(a) (sizeof((a))/sizeof((a)[0]))

using namespace std;

struct ShaderSource
{
	std::vector<const char *> vertex;
	std::vector<const char *> fragment;
	std::vector<const char *> geometry;
	std::vector<const char *> compute;
	std::vector<const char *> tess_control;
	std::vector<const char *> tess_eval;
	std::vector<const char *> feedback_varyings;
};

static unsigned int
compile_src(const std::vector<const char *> &src, int type, std::vector<const char *> const *orig_names)
{
	GLuint shader_obj = glCreateShader(type);

	glShaderSource(shader_obj, src.size(), (const char **) &src[0], NULL);
	glCompileShader(shader_obj);

	int success;
	glGetShaderiv(shader_obj, GL_COMPILE_STATUS, &success);
	if(!success) {
		char infolog[2048];
		glGetShaderInfoLog(shader_obj, sizeof infolog, NULL, infolog);
		std::cerr << "error compiling shader: ";
		if (orig_names)
			for (auto& i : *orig_names)
				std::cerr << i << ", ";
		std::cerr << std::endl << infolog << std::endl;
	}

	return shader_obj;
}

static unsigned int
compile_shader(const ShaderSource &src, const ShaderSource *src_orig)
{
	GLuint prog = glCreateProgram();

	unsigned int vert = 0, frag = 0, geom = 0, comp = 0, tess_cntrl = 0, tess_eval = 0;

	if(src.vertex.size())
		glAttachShader(prog, vert = compile_src(src.vertex, GL_VERTEX_SHADER, src_orig ? &src_orig->vertex : nullptr));
	if(src.fragment.size())
		glAttachShader(prog, frag = compile_src(src.fragment, GL_FRAGMENT_SHADER, src_orig ? &src_orig->fragment : nullptr));
	if(src.geometry.size())
		glAttachShader(prog, geom = compile_src(src.geometry, GL_GEOMETRY_SHADER, src_orig ? &src_orig->geometry : nullptr));
	if(src.compute.size())
		glAttachShader(prog, comp = compile_src(src.compute, GL_COMPUTE_SHADER, src_orig ? &src_orig->compute : nullptr));
	if(src.tess_control.size())
		glAttachShader(prog, tess_cntrl = compile_src(src.tess_control, GL_TESS_CONTROL_SHADER, src_orig ? &src_orig->tess_control : nullptr));
	if(src.tess_eval.size())
		glAttachShader(prog, tess_eval = compile_src(src.tess_eval, GL_TESS_EVALUATION_SHADER, src_orig ? &src_orig->tess_eval : nullptr));
	if(src.feedback_varyings.size()) {
		glTransformFeedbackVaryings(prog, src.feedback_varyings.size(),
				(const char **)&src.feedback_varyings[0], GL_INTERLEAVED_ATTRIBS);
	}

	glLinkProgram(prog);

	int success;
	glGetProgramiv(prog, GL_LINK_STATUS, &success);
	if(!success) {
		char infolog[2048];
		glGetProgramInfoLog(prog, sizeof infolog, NULL, infolog);
		std::cerr << "error linking shaderprogram: ";
		if (src_orig)
			for (auto x : { &src_orig->vertex, &src_orig->fragment, &src_orig->geometry, &src_orig->compute, &src_orig->tess_control, &src_orig->tess_eval })
				for (auto& i : *x)
					std::cerr << i << ", ";
		std::cerr << std::endl << infolog << std::endl;
	}

	glDeleteShader(vert);
	glDeleteShader(frag);
	glDeleteShader(geom);
	glDeleteShader(comp);
	glDeleteShader(tess_cntrl);
	glDeleteShader(tess_eval);

	return prog;
}

void ShaderManager::
reload_shader()
{
	cout << "reloading all shader!" << endl;
	glUseProgram(0);
	for(auto it = shaders.begin(); it != shaders.end(); ++it) {
		delete it->second;
	}
	shaders.clear();
}

Shader::
Shader(const ShaderSource &src, const ShaderSource *src_orig)
{
	program = compile_shader(src, src_orig);

	int num_uniforms;
	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &num_uniforms);

	for(int i = 0; i < num_uniforms; i++) {
		char buf[128];
		GLenum type;
		int size;
		glGetActiveUniform(program, i, sizeof buf, 0, &size, &type, buf);
		uniforms[buf] = Uniform(glGetUniformLocation(program, buf), type);
	}
}

Shader & ShaderManager::
operator[](const string &str)
{
	auto it = shaders.find(str);
	if(it != shaders.end())
		return *it->second;

	if(!load_shader(str))
		return *shaders[str];

	cerr << "error: could not load shader \"" << str << "\"" << std::endl;
	auto it_err = shaders.find("error_shader");
	if(it_err != shaders.end())
		return *it_err->second;
	return *(shaders[str] = new Shader());
}

int ShaderManager::
load_shader(const std::string &path)
{
	std::cout << "loading shader \"" << path << "\"" << std::endl;
	auto it = program_definitions.find(path);
	if(it == program_definitions.end())
		return 1;

	auto &p = it->second;

	ShaderSource src;
	ShaderSource srcOrig;

	for(auto i: p.files_vertex) {
		if(auto x = file_read(i))
			src.vertex.push_back(x), srcOrig.vertex.push_back(i);
	}
	for(auto i: p.files_fragment) {
		if(auto x = file_read(i))
			src.fragment.push_back(x), srcOrig.fragment.push_back(i);
	}
	for(auto i: p.files_geom) {
		if(auto x = file_read(i))
			src.geometry.push_back(x), srcOrig.geometry.push_back(i);
	}
	for(auto i: p.feedback_varyings) {
		src.feedback_varyings.push_back(i);
	}

	std::string defines;

	ShaderSource src_patched = src; /* copy which is modified to
											   make delete[] still work
											   later */
	for(auto i: { &src_patched.vertex, &src_patched.fragment, &src_patched.geometry,
			&src_patched.compute, &src_patched.tess_control, &src_patched.tess_eval } ) {
		if(i->size() == 0)
			continue;

		cg_assert(i->at(0));
		if(strncmp(i->at(0), "#version ", strlen("#version "))) {
			cerr << "warning, #version statement missing" << endl;
			continue;
		}

		/* XXX: assumes that only non-const char is stored... */
		char *offset = const_cast<char *>(strchr(i->at(0), '\n')); /* make newline
														  after #version
														  xxx to 0 */
		*offset = 0;

		const char *shader_header[] = { i->at(0), "\n", defines.c_str(), "#line 2\n" };

		i->at(0) = offset + 1;
		i->insert(i->begin(), shader_header, shader_header + LENGTH(shader_header));
	}

	shaders[path] = new Shader(src_patched, &srcOrig);

	for(auto i: src.vertex)
		delete[] i;
	for(auto i: src.fragment)
		delete[] i;
	for(auto i: src.geometry)
		delete[] i;
	for(auto i: src.compute)
		delete[] i;
	for(auto i: src.tess_control)
		delete[] i;
	for(auto i: src.tess_eval)
		delete[] i;
	/* feedback varyings are const char, do not delete! */

	return 0;
}
