#include <cglib/rt/intersection_tests.h>
#include <cglib/rt/light.h>
#include <cglib/rt/object.h>
#include <cglib/rt/ray.h>
#include <cglib/rt/raytracing_context.h>
#include <cglib/rt/render_data.h>
#include <cglib/rt/renderer.h>
#include <cglib/rt/sampling_patterns.h>
#include <cglib/rt/scene.h>
#include <cglib/rt/texture.h>
#include <cglib/rt/host_render.h>

#include <cglib/core/glmstream.h>
#include <cglib/core/heatmap.h>
#include <cglib/core/image.h>
#include <cglib/core/parameters.h>
#include <cglib/core/thread_local_data.h>
#include <cglib/core/thread_local_data.h>

#include <cglib/core/assert.h>
#include <cglib/core/better_sprintf.h>
#include <cmath>
#include <sstream>
#include <chrono>

using std::cout;
using std::cerr;
using std::endl;

static const std::string image_prefix = "assignment_images/";

enum {
	RES_WIDTH = 256,
	RES_HEIGHT = 256
};

enum {
	NORMAL_MAPPING    = (1 << 3),
	TRANSFORM_OBJECTS = (1 << 4),
};

enum {
	SCENE_GO,
	SCENE_POOL,
	SCENE_ALIASING_PLANE,
	SCENE_TEXTURED_SPHERE,
	SCENE_LAST
};

std::string get_scene_prefix(int scene)
{
	switch(scene) {
		case SCENE_GO: return "goboard";
		case SCENE_POOL: return "pooltable";
		case SCENE_ALIASING_PLANE: return "plane";
		case SCENE_TEXTURED_SPHERE: return "sphere";
		default: cg_assert(!"invalid scene");
	}
	return "";
}

/*
 * This is the main rendering kernel.
 *
 * It is supposed to compute the RGB color of the given pixel (x,y).
 *
 * RenderData contains data relevant for the computation of the color
 * for one pixel. Thread-local data is referenced by this struct, aswell. The
 * pointer tld is guaranteed to be valid (not nullptr).
 */
glm::vec3 render_pixel(int x, int y, RaytracingContext const& context, RenderData &data)
{
	cg_assert(data.tld);

	std::vector<glm::vec2> samples;
	int spp = data.context.params.spp;

	if(spp > 1) {
		int grid_size = int(sqrtf(static_cast<float>(spp)));
		if(data.context.params.stratified)
			generate_stratified_samples(&samples, grid_size, grid_size, data.tld);
		else
			generate_random_samples(&samples, grid_size, grid_size, data.tld);
		glm::vec3 accum(0.0f);

		for(size_t i = 0; i < samples.size(); i++) {
			float fx = float(x) + samples[i].x;
			float fy = float(y) + samples[i].y;

			data.x = fx;
			data.y = fy;

			Ray ray = createPrimaryRay(data, fx, fy);
			accum += trace_recursive(data, ray, 0/*depth*/);
		}

		return accum / float(samples.size());
	}
	else {
		float fx = float(x) + 0.5f;
		float fy = float(y) + 0.5f;

		data.x = fx;
		data.y = fy;

		Ray ray = createPrimaryRay(data, fx, fy);
		return trace_recursive(data, ray, 0/*depth*/);
	}
}

static void
render_image(
		const std::string &output_name,
		int params, int scene, RaytracingParameters::RenderMode render_mode,
		TextureFilterMode tex_filter_mode,
		TextureWrapMode tex_wrap_mode = REPEAT,
		int max_depth = 1)
{
    RaytracingContext context;
	context.params.max_depth         = max_depth;
	context.params.image_width       = RES_WIDTH;
	context.params.image_height      = RES_HEIGHT;
	context.params.output_file_name  = output_name;
	context.params.interactive       = 0;
	context.params.spp               = 1;
	context.params.render_mode       = render_mode;
	context.params.tex_filter_mode   = tex_filter_mode;
	context.params.tex_wrap_mode     = tex_wrap_mode;
	context.params.normal_mapping    = !!(params & NORMAL_MAPPING);
	context.params.transform_objects = !!(params & TRANSFORM_OBJECTS);

	// hack because i'm lazy, small performance optimization
	if(render_mode == RaytracingParameters::NORMAL) {
		context.params.reflection = false;
		context.params.shadows = false;
		max_depth = 1;
	}

	switch(scene) {
	case SCENE_GO:
		context.scenes.insert({"scene", std::make_shared<GoBoardScene>(context.params)});
		break;
	case SCENE_POOL:
		context.scenes.insert({"scene", std::make_shared<PoolTableScene>(context.params)});
		break;
	case SCENE_ALIASING_PLANE:
		context.scenes.insert({"scene", std::make_shared<AliasingPlaneScene>(context.params)});
		break;
	case SCENE_TEXTURED_SPHERE:
		context.scenes.insert({"scene", std::make_shared<TexturedSphereScene>(context.params)});
		break;
	default:
		cg_assert(!"invalid scene");
	}
	context.scene = context.scenes["scene"];

	HostRender::run(context, render_pixel);
}

static void
create_images_a(int scene)
{
	render_image(image_prefix+get_scene_prefix(scene)+"_after_a_zero.tga", 0,
		scene, RaytracingParameters::RenderMode::RECURSIVE,
		TextureFilterMode::NEAREST, TextureWrapMode::ZERO);
}

static void
create_images_b(int scene)
{
	render_image(image_prefix+get_scene_prefix(scene)+"_after_b_clamp.tga", 0,
		scene, RaytracingParameters::RenderMode::RECURSIVE,
		TextureFilterMode::NEAREST, TextureWrapMode::CLAMP);
	render_image(image_prefix+get_scene_prefix(scene)+"_after_b_repeat.tga", 0,
		scene, RaytracingParameters::RenderMode::RECURSIVE,
		TextureFilterMode::NEAREST, TextureWrapMode::REPEAT);
}

static void
create_images_c(int scene)
{
	render_image(image_prefix+get_scene_prefix(scene)+"_after_c.tga", 0,
		scene, RaytracingParameters::RenderMode::RECURSIVE,
		TextureFilterMode::BILINEAR, TextureWrapMode::REPEAT);
}

static void
create_images_d(int scene)
{
	render_image(image_prefix+get_scene_prefix(scene)+"_after_d.tga", 0,
		scene, RaytracingParameters::RenderMode::RECURSIVE,
		TextureFilterMode::TRILINEAR, TextureWrapMode::REPEAT);
	render_image(image_prefix+get_scene_prefix(scene)+"_after_d_debugmip.tga", 0,
		scene, RaytracingParameters::RenderMode::RECURSIVE,
		TextureFilterMode::DEBUG_MIP, TextureWrapMode::REPEAT);
	render_image(image_prefix+get_scene_prefix(scene)+"_after_d_dudv.tga", 0,
		scene, RaytracingParameters::RenderMode::DUDV,
		TextureFilterMode::TRILINEAR, TextureWrapMode::REPEAT);
}

static void
create_images_e(int scene)
{
	render_image(image_prefix+get_scene_prefix(scene)+"_after_e.tga",
		TRANSFORM_OBJECTS,
		scene, RaytracingParameters::RenderMode::RECURSIVE,
		TextureFilterMode::NEAREST, TextureWrapMode::REPEAT);
}

static void
create_images_f(int scene)
{
	render_image(image_prefix+get_scene_prefix(scene)+"_after_f.tga",
		NORMAL_MAPPING | TRANSFORM_OBJECTS,
		scene, RaytracingParameters::RenderMode::RECURSIVE,
		TextureFilterMode::NEAREST, TextureWrapMode::REPEAT);
}

static void
create_images()
{
	create_images_a(SCENE_ALIASING_PLANE);
	create_images_a(SCENE_GO);
	
	create_images_b(SCENE_ALIASING_PLANE);
	create_images_c(SCENE_ALIASING_PLANE);
	create_images_c(SCENE_GO);

	create_images_d(SCENE_ALIASING_PLANE);
	create_images_d(SCENE_GO);
	create_images_d(SCENE_TEXTURED_SPHERE);

	create_images_e(SCENE_GO);
	create_images_e(SCENE_POOL);

	create_images_f(SCENE_GO);
	create_images_f(SCENE_TEXTURED_SPHERE);

	ImageTexture checker_board("assets/checker.tga", TRILINEAR, ZERO, 1.0f);
	const auto &mip_levels = checker_board.get_mip_levels();
	for(int i = 0; i < (int)mip_levels.size(); i++) {
		std::string filename = sprintf(image_prefix+"%s%02d.tga", "miplevel", i);
		mip_levels[i]->saveTGA(filename, 1.0f);
	}
}

int
main(int argc, char const**argv)
{
    RaytracingContext context;
    if (!context.params.parse_command_line(argc, argv)) {
        std::cerr << "invalid command line argument" << std::endl;
        return -1;
    }

	if(context.params.create_images) {
		create_images();
		return 0;
	}

    context.scenes.insert({"go_board", std::make_shared<GoBoardScene>(context.params)});
    context.scenes.insert({"pool_table", std::make_shared<PoolTableScene>(context.params)});
    context.scenes.insert({"textured_sphere", std::make_shared<TexturedSphereScene>(context.params)});
    context.scenes.insert({"aliasing_plane", std::make_shared<AliasingPlaneScene>(context.params)});
	context.scene = context.scenes["aliasing_plane"];

	return HostRender::run(context, render_pixel);
}

// CG_REVISION dfce8167a2ff99a11f4b6ce59777acd3a80ed38f
