#pragma once

#include <cglib/rt/texture.h>
#include <cglib/rt/epsilon.h>

#include <cglib/core/parameters.h>

struct CTwBar;

/*
 * Raytracing parameters.
 *
 * Have a look at cglib/parameters.h to see which parameters are built-in already.
 * Feel free to add more parameters here. You can add them to the AntTweakBar gui,
 * aswell, to make interactive tweaking easier.
 */
class RaytracingParameters : public Parameters
{
public:
	enum RenderMode {
		RECURSIVE,
		DESATURATE,
		NUM_RAYS,
		NORMAL,
		TIME,
		DUDV,
	};

	enum Scene {
		GO_BOARD,
		POOL_TABLE,
		TEXTURED_SPHERE,
		ALIASING_PLANE,
	};

	RenderMode render_mode  = RECURSIVE;
	bool diffuse_white_mode = false;
	int max_depth           = 4;
	bool shadows            = true;
	bool ambient            = true;
	bool diffuse            = true;
	bool specular           = true;
	bool reflection         = true;
	bool transmission       = true;
	bool dispersion         = false;
	bool fresnel            = false;
	float scale_render_time = 10.0f;
	float ray_epsilon       = 7.f*1e-3f;
	float fovy              = 45.0f;

	bool stratified = false;

	bool normal_mapping = false;
	bool transform_objects = true;
	std::uint32_t spp = 1; // number of samples per pixel

	TextureFilterMode tex_filter_mode = TextureFilterMode::NEAREST;
	TextureWrapMode tex_wrap_mode = TextureWrapMode::REPEAT;

	Scene scene = ALIASING_PLANE;

	virtual bool derived_change_requires_restart(Parameters const& old_) const final;
	virtual void derived_gui_setup(CTwBar *main_bar) override final;
};
