#include <cglib/rt/intersection_tests.h>
#include <cglib/rt/ray.h>
#include <cglib/rt/raytracing_context.h>
#include <cglib/rt/render_data.h>
#include <cglib/rt/renderer.h>
#include <cglib/rt/scene.h>
#include <cglib/rt/host_render.h>

#include <cglib/core/glmstream.h>
#include <cglib/core/heatmap.h>
#include <cglib/core/image.h>
#include <cglib/core/parameters.h>
#include <cglib/core/thread_local_data.h>
#include <cglib/core/thread_local_data.h>

#include <glm/gtc/matrix_transform.hpp>

#include <cglib/core/assert.h>
#include <cmath>
#include <sstream>
#include <chrono>

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
	float fx = x + 0.5f;
	float fy = y + 0.5f;

	data.x = fx;
	data.y = fy;

	Ray ray = createPrimaryRay(data, fx, fy);
	return trace_recursive(data, ray, 0/*depth*/);
}

int
main(int argc, char const**argv)
{
    RaytracingContext context;
    if (!context.params.parse_command_line(argc, argv)) {
        std::cerr << "invalid command line argument" << std::endl;
        return -1;
    }

    context.scenes.insert({"sphere_portrait", std::make_shared<SpherePortrait>(context.params)});
    context.scenes.insert({"cornell_box", std::make_shared<CornellBox>(context.params)});
	context.scene = context.scenes["cornell_box"];

	return HostRender::run(context, render_pixel);
}

// CG_REVISION b27b5056a58a9221eb3999b4ae8f60f5ab4868a3
