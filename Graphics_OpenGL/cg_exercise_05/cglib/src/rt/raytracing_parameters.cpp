#include <cglib/rt/raytracing_parameters.h>
#include <cglib/core/assert.h>

#include <AntTweakBar.h>

#define LENGTH(a) ((sizeof(a))/sizeof((a)[0]))

static TwEnumVal tex_filter_enum[] = {
	{ NEAREST,   "Nearest"   },
	{ BILINEAR,  "Bilinear"  },
	{ TRILINEAR, "Trilinear" },
	{ DEBUG_MIP, "Debug Mip" }
}; 

static TwEnumVal tex_wrap_enum[] = {
	{ REPEAT, "Repeat" },
	{ CLAMP,  "Clamp"  },
	{ ZERO,   "Zero"   }
}; 

static TwEnumVal render_mode_enum[] = {
	{ RaytracingParameters::RECURSIVE,            "Recursive"               },
	{ RaytracingParameters::DESATURATE,           "Desaturate"              },
	{ RaytracingParameters::NUM_RAYS,             "Num rays cast"           },
	{ RaytracingParameters::NORMAL,               "Normal"                  },
	{ RaytracingParameters::TIME,                 "Time"                    },
	{ RaytracingParameters::DUDV,                 "dudv"                    },
	{ RaytracingParameters::BVH_TIME,             "BVH Intersection Time"   },
	{ RaytracingParameters::AABB_INTERSECT_COUNT, "AABB Intersection Count" },
};

static TwEnumVal scene_enum[] = {
	{ RaytracingParameters::MONKEY,            "Monkey"          },
	{ RaytracingParameters::SPONZA,            "Sponza"          },
	{ RaytracingParameters::POOL_TABLE,        "Pool Table"      },
};

static void TW_CALL
eye_sep_set(void const* value, void* )
{
	auto cam = Camera::get_active();
	if(cam)
		cam->set_eye_separation(*reinterpret_cast<float const*>(value));
}

static void TW_CALL
eye_sep_get(void* value, void* )
{
	auto cam = Camera::get_active();
	if(cam)
		*reinterpret_cast<float*>(value) = cam->get_eye_separation();
}

void RaytracingParameters::
derived_gui_setup(TwBar* bar)
{
	cg_assert(bar);

	TwType render_mode_type = TwDefineEnum("Render Mode",         render_mode_enum, LENGTH(render_mode_enum));
	TwType tex_filter_type  = TwDefineEnum("Texture filter Mode", tex_filter_enum,  LENGTH(tex_filter_enum));
	TwType tex_wrap_type    = TwDefineEnum("Texture Wrap Mode",   tex_wrap_enum,    LENGTH(tex_wrap_enum));

	TwType scene_type = TwDefineEnum("Scene", scene_enum, LENGTH(scene_enum));
	TwAddVarRW(bar, "scene", scene_type, &scene, "label='Scene' group='Rendering Settings'");

	TwAddVarRW(bar, "render_mode",  render_mode_type, &render_mode,  "label='Render Mode' group='Rendering Settings'");
	TwAddVarRW(bar, "diffuse_white_mode", TW_TYPE_BOOLCPP, &diffuse_white_mode, "label='Diffuse White' group='Shading Settings'");
	TwAddVarRW(bar, "max_depth",    TW_TYPE_INT32,    &max_depth,    "label='Max Depth' group='Rendering Settings'");
	TwAddVarRW(bar, "shadows",      TW_TYPE_BOOLCPP,  &shadows,      "label='Shadows' group='Shading Settings'");
	TwAddVarRW(bar, "ambient",      TW_TYPE_BOOLCPP,  &ambient,      "label='Ambient Lighting' group='Shading Settings'");
	TwAddVarRW(bar, "diffuse",      TW_TYPE_BOOLCPP,  &diffuse,      "label='Diffuse Lighting' group='Shading Settings'");
	TwAddVarRW(bar, "specular",     TW_TYPE_BOOLCPP,  &specular,     "label='Specular Lighting' group='Shading Settings'");
	TwAddVarRW(bar, "reflection",   TW_TYPE_BOOLCPP,  &reflection,   "label='Reflection' group='Shading Settings'");
	TwAddVarRW(bar, "transform_objects", TW_TYPE_BOOLCPP, &transform_objects,  "label='Transform Objects' group='Shading Settings'");
	TwAddVarRW(bar, "normal_mapping", TW_TYPE_BOOLCPP, &normal_mapping,  "label='Normal Mapping' group='Shading Settings'");
	TwAddVarRW(bar, "tex_filter",     tex_filter_type, &tex_filter_mode, "label='Texture filter' group='Texturing Settings'");
	TwAddVarRW(bar, "tex_wrap",       tex_wrap_type,   &tex_wrap_mode,   "label='Texture Wrap' group='Texturing Settings'");
	TwAddVarRW(bar, "indirect",          TW_TYPE_BOOLCPP,  &indirect,      "label='Indirect Illumination' group='Shading Settings'");
	TwAddVarRW(bar, "ambient_occlusion", TW_TYPE_BOOLCPP,  &ao,            "label='Ambient Occlusion' group='Shading Settings'");
	TwAddVarRW(bar, "depth_of_field",    TW_TYPE_BOOLCPP,  &dof,           "label='Depth of Field' group='Shading Settings'");
	TwAddVarRW(bar, "soft_shadows",      TW_TYPE_BOOLCPP,  &soft_shadow,   "label='Soft Shadows' group='Shading Settings'");
	TwAddVarRW(bar, "indirect_rays",     TW_TYPE_INT32,    &indirect_rays,  "label='# Indirect Rays' help='Number of indirect illumination rays' group='Shading Settings' min=0");
	TwAddVarRW(bar, "ao_rays",           TW_TYPE_INT32,    &ao_rays,        "label='# AO Rays' help='Number of ambient occlusion rays' group='Shading Settings' min=0");
	TwAddVarRW(bar, "half_ao_radius",    TW_TYPE_FLOAT,    &half_ao_radius, "label='Distance to half occlusion' group='Shading Settings' min=0");
	TwAddVarRW(bar, "dof_rays",          TW_TYPE_INT32,    &dof_rays,       "label='# DOF Rays' help='Number of depth of field rays' group='Shading Settings' min=0");
	TwAddVarRW(bar, "lens_radius",       TW_TYPE_FLOAT,    &lens_radius,    "label='Lens Radius' group='Shading Settings' min=0");
	TwAddVarRW(bar, "focal_length",      TW_TYPE_FLOAT,    &focal_length,   "label='Focal Length' group='Shading Settings' min=0");
	TwAddVarRW(bar, "shadow_rays",       TW_TYPE_INT32,    &shadow_rays,    "label='# Shadow Rays' help='Number of shadow rays' group='Shading Settings' min=0");
	TwAddVarRW(bar, "disable_direct",    TW_TYPE_BOOLCPP,  &disable_direct, "label='Disable Direct Lighting' group='Shading Settings'");
	TwAddVarRW(bar, "stratified", TW_TYPE_BOOLCPP, &stratified, "label='Stratified Sampling' group='Rendering Settings'");
	TwAddVarRW(bar, "ray_epsilon", TW_TYPE_FLOAT, &ray_epsilon, "label='Ray Epsilon' group='Shading Settings' min=0.0 step=0.0001");

	TwAddVarRW(bar, "stereo",            TW_TYPE_BOOL8,  &stereo,            "label='Stereo Rendering' group='General Settings'");
	TwAddVarCB(bar, "eye_separation",    TW_TYPE_FLOAT,  eye_sep_set, eye_sep_get, nullptr, "label='Eye Separation' group='General Settings' min=0 step=0.01");
	TwAddVarRW(bar, "fovy",              TW_TYPE_FLOAT,  &fovy,              "label='FovY' group='General Settings' min=1.0 step=0.1");
	TwAddVarRW(bar, "exposure",          TW_TYPE_FLOAT,  &exposure,          "label='Exposure' group='General Settings' min=0 step=0.01");
	TwAddVarRW(bar, "gamma",             TW_TYPE_FLOAT,  &gamma,             "label='Gamma' group='General Settings' min=0 step=0.01");
	TwAddVarRW(bar, "scale_render_time", TW_TYPE_FLOAT,  &scale_render_time, "label='Scale render time' group='General Settings' min=0 step=0.01");
	TwAddVarRO(bar, "num_threads",       TW_TYPE_UINT32, &num_threads,       "label='Render threads' group='General Settings'");
	TwAddVarRO(bar, "image_width",       TW_TYPE_UINT32, &image_width,       "label='Image width' group='General Settings'");
	TwAddVarRO(bar, "image_height",      TW_TYPE_UINT32, &image_height,      "label='Image height' group='General Settings'");
	TwAddVarRW(bar, "spp", TW_TYPE_UINT32, &spp, "label='Pixel Samples' group='Rendering Settings' min=1");
}

bool RaytracingParameters::
derived_change_requires_restart(Parameters const& old_) const
{
	auto const* old = dynamic_cast<RaytracingParameters const*>(&old_);
	cg_assert(old); // this would be a serious implementation error

	const bool restart = false
		|| (ambient           != old->ambient)
		|| (diffuse           != old->diffuse)
		|| (dispersion        != old->dispersion)
		|| (fovy              != old->fovy)
		|| (fresnel           != old->fresnel)
		|| (max_depth         != old->max_depth)
		|| (ray_epsilon       != old->ray_epsilon)
		|| (reflection        != old->reflection)
		|| (render_mode       != old->render_mode)
		|| (diffuse_white_mode       != old->diffuse_white_mode)
		|| (scale_render_time != old->scale_render_time)
		|| (scene             != old->scene)
		|| (shadows           != old->shadows)
		|| (specular          != old->specular)
		|| (stereo            != old->stereo)
		|| (tex_filter_mode   != old->tex_filter_mode)
		|| (tex_wrap_mode     != old->tex_wrap_mode)
		|| (transmission      != old->transmission)
		|| (indirect          != old->indirect)
		|| (ao                != old->ao)
		|| (dof               != old->dof)
		|| (soft_shadow       != old->soft_shadow)
		|| (indirect_rays     != old->indirect_rays)
		|| (ao_rays           != old->ao_rays)
		|| (half_ao_radius    != old->half_ao_radius)
		|| (dof_rays          != old->dof_rays)
		|| (lens_radius       != old->lens_radius)
		|| (focal_length      != old->focal_length)
		|| (shadow_rays       != old->shadow_rays)
		|| (disable_direct    != old->disable_direct)
		|| (stratified        != old->stratified)
		|| (normal_mapping    != old->normal_mapping)
		|| (transform_objects != old->transform_objects)
		|| (spp               != old->spp)
		|| (filtered_envmap   != old->filtered_envmap)
		|| (num_triangles     != old->num_triangles)
		;

	return restart;
}

