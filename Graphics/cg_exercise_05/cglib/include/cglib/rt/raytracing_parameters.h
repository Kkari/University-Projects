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
		BVH_TIME,
		AABB_INTERSECT_COUNT,
	};

	enum Scene {
		MONKEY,
		SPONZA,
		TRIANGLES,
			POOL_TABLE,
//			GO_BOARD,
	};

	RenderMode render_mode  = RECURSIVE;
	bool diffuse_white_mode = false;
	int max_depth           = 1;
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

	bool stratified = true;

	bool normal_mapping = false;
	bool transform_objects = true;
	std::uint32_t spp = 1; // number of samples per pixel

	bool filtered_envmap = false;
	int num_triangles = 5;

	bool indirect        = false;
	bool ao              = false;
	bool dof             = false;
	bool soft_shadow     = false;
	int indirect_rays    = 64;
	int ao_rays          = 64;
	float half_ao_radius = 16.0f;
	int dof_rays         = 32;
	float lens_radius    = 0.2f;
	float focal_length   = 7.5f;
	int shadow_rays      = 32;
	bool disable_direct  = false;

	TextureFilterMode tex_filter_mode = TextureFilterMode::TRILINEAR;
	TextureWrapMode tex_wrap_mode = TextureWrapMode::REPEAT;

	Scene scene = MONKEY;

	virtual bool derived_change_requires_restart(Parameters const& old_) const final;
	virtual void derived_gui_setup(CTwBar *main_bar) override final;
};
