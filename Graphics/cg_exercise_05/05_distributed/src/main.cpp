#include <cglib/rt/bvh.h>
#include <cglib/rt/intersection_tests.h>
#include <cglib/rt/ray.h>
#include <cglib/rt/raytracing_context.h>
#include <cglib/rt/render_data.h>
#include <cglib/rt/renderer.h>
#include <cglib/rt/scene.h>
#include <cglib/rt/sampling_patterns.h>
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

using std::cout;
using std::cerr;
using std::endl;

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
			accum += trace_recursive_with_lens(data, ray, 0/*depth*/);
		}

		return accum / float(samples.size());
	}
	else {
		float fx = float(x) + 0.5f;
		float fy = float(y) + 0.5f;

		data.x = fx;
		data.y = fy;

		Ray ray = createPrimaryRay(data, fx, fy);
		return trace_recursive_with_lens(data, ray, 0/*depth*/);
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
	
	// Sponza is created lazily when selected in GUI (see host_render.cpp).
	context.scenes.insert({ "monkey", std::make_shared<MonkeyScene>(context.params) });
	context.scenes.insert({ "pool_table", std::make_shared<PoolTableScene>(context.params) });
	context.scene = context.scenes["monkey"];

	return HostRender::run(context, render_pixel);
}
// CG_REVISION 11b5702e34b37bad947c1f3cb241cd9fe1dc6bc4
