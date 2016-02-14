#include <cglib/core/glheaders.h>
#include <cglib/core/assert.h>
#include <cglib/gl/fbo.h>

#include <iostream>

static GLuint
gen_renderbuffer(GLenum internal_format, int w, int h)
{
	GLuint rb;
	glGenRenderbuffers(1, &rb);
	glBindRenderbuffer(GL_RENDERBUFFER, rb);
	glRenderbufferStorage(GL_RENDERBUFFER, internal_format, w, h);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	return rb;
}

static int mip_level_count(int w, int h)
{
	int c = 0;
	for (int d = w > h ? w : h; d > 0; d /= 2)
		++c;
	return c;
}

static GLuint
gen_texture(GLenum internal_format, int w, int h, bool mip)
{
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	int levels = mip ? mip_level_count(w, h) : 1;
	glTexStorage2D(GL_TEXTURE_2D, levels, internal_format, w, h);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mip ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	return tex;
}

static GLuint
gen_fbo()
{
	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	return fbo;
}

static void
check_framebuffer_complete()
{
# define ERROR_CASE(a)                                 \
	case a:                                            \
		std::cerr << "error setting up fbo: \"" #a << std::endl; \
		break;

	switch(glCheckFramebufferStatus(GL_FRAMEBUFFER)) {

	ERROR_CASE(GL_FRAMEBUFFER_UNDEFINED);
	ERROR_CASE(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
	ERROR_CASE(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER);
	ERROR_CASE(GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS);
	ERROR_CASE(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);
	ERROR_CASE(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE);
	ERROR_CASE(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER);
	ERROR_CASE(GL_FRAMEBUFFER_UNSUPPORTED);

	case GL_FRAMEBUFFER_COMPLETE:
		break;
	default:
		std::cerr << "this should not happen" << std::endl;
		break;
	}
#undef ERROR_CASE
}

void FBO::
init_fbo(const FBOInit &fbo_init)
{
	int w = width, h = height;
	fbo = gen_fbo();
	bind();

	GLenum active_attachments[FBOInit::MAX_COLOR_ATTACHMENTS];
	GLenum f;
	for(int i = 0; i < FBOInit::MAX_COLOR_ATTACHMENTS; i++) {
		if ((f = fbo_init.color[i])) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
				GL_TEXTURE_2D, tex_color[i] = gen_texture(f, w, h, fbo_init.mipchain), 0);
			active_attachments[i] = GL_COLOR_ATTACHMENT0 + i;
		}
		else
			active_attachments[i] = GL_NONE;
	}
	glDrawBuffers(FBOInit::MAX_COLOR_ATTACHMENTS, active_attachments);

	if((f = fbo_init.rb)) {
		GLenum attach = GL_DEPTH_ATTACHMENT;
		if(f == GL_DEPTH24_STENCIL8 || f == GL_DEPTH32F_STENCIL8) {
			attach = GL_DEPTH_STENCIL_ATTACHMENT;
		}

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, attach, GL_RENDERBUFFER,
				rb = gen_renderbuffer(f, w, h));
	}

	if((f = fbo_init.depth)) {
		glBindTexture(GL_TEXTURE_2D, tex_depth = gen_texture(f, w, h, false));
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		//float l_ClampColor[] = {0.0, 0.0, 0.0, 0.0};
		//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, l_ClampColor);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tex_depth, 0);
	}

	check_framebuffer_complete();
	unbind();
}

void FBO::
bind()
{
	cg_assert(!is_bound);
	is_bound = 1;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, previos_fbo_binding);
	glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, previos_fbo_binding + 1);
	glGetIntegerv(GL_VIEWPORT, old_viewport);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0, 0, this->width, this->height);
}

void FBO::
unbind()
{
	cg_assert(is_bound);
	is_bound = 0;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, previos_fbo_binding[0]);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, previos_fbo_binding[1]);
	glViewport(old_viewport[0], old_viewport[1], old_viewport[2], old_viewport[3]);
}
