#pragma once

#include <cstring>

struct FBOInit
{
	enum { MAX_COLOR_ATTACHMENTS = 8 };
	unsigned int color[MAX_COLOR_ATTACHMENTS];
	unsigned int depth;
	unsigned int rb;
	int width, height;
	bool mipchain;

	FBOInit(int w, int h)
	{
		std::memset(this, 0, sizeof(*this));
		width = w;
		height = h;
	}

	FBOInit & attach_color(int idx, unsigned int type) {
		color[idx] = type;
		return *this;
	}

	FBOInit & attach_depth(unsigned int type) {
		depth = type;
		return *this;
	}

	FBOInit & attach_rb(unsigned int type) {
		rb = type;
		return *this;
	}

	FBOInit & create_mipchain(bool create = true) {
		mipchain = create;
		return *this;
	} 
};

struct FBO
{
	int width, height;
	unsigned int fbo;
	unsigned int tex_color[FBOInit::MAX_COLOR_ATTACHMENTS];
	unsigned int rb;
	unsigned int tex_depth;
	int old_viewport[4];
	int previos_fbo_binding[2];
	unsigned int is_bound;

	FBO(const FBOInit &fbo_init)
	{
		std::memset(this, 0, sizeof(*this));
		width = fbo_init.width;
		height = fbo_init.height;
		init_fbo(fbo_init);
	}

	void bind();
	void unbind();

protected:
	void init_fbo(const FBOInit &fbo_init);
};
