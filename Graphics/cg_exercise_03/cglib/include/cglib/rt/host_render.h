#pragma once

#include <cglib/core/gui.h>
#include <cglib/core/thread_local_data.h>
#include <cglib/core/thread_pool.h>
#include <cglib/core/timer.h>
#include <cglib/core/stereo.h>

#include <cglib/rt/raytracing_context.h>
#include <cglib/rt/scene.h>
#include <cglib/rt/render_data.h>

#include <cglib/core/assert.h>
#include <chrono>
#include <functional>
#include <iostream>
#include <mutex>

static std::mutex mutex;

struct RenderData;

/*
 * Use this class to render on the host (so not primarily with OpenGL), in an image order fashion.
 * Will use a thread pool to launch multiple threads in parallel.
 */
class HostRender
{
	public:
		/*
		 * The parameters to the pixel function are:
		 * int x, int y         (pixel coordinates)
		 * centext const&       (The current context (scene+parameters)).
		 */
		typedef std::function<glm::vec3(int, int, RaytracingContext const&, RenderData &)> PixelFunc;

		static int run(RaytracingContext& context, 
				       PixelFunc const& render_pixel, 
					   int kill_timeout_seconds = 0,
					   std::function<void()> const& render_overlay = []() {} );

	private:
		typedef std::function<glm::vec3(int, int, RaytracingContext const&, ThreadLocalData*)> PixelFuncRaw;
		static void generate_tile_idx(int num_tiles_x, int num_tiles_y, std::vector<glm::ivec2>* tile_idx);
		static int run_interactive(RaytracingContext& context, PixelFuncRaw const& render_pixel, 
			std::function<void()> const& render_overlay = []() {} );
		static int run_noninteractive(RaytracingContext& context, 
			PixelFuncRaw const& render_pixel,
			int kill_timeout_seconds);
		static void launch(Image* fb, ThreadPool& thread_pool, RaytracingContext const* context, std::vector<glm::ivec2>* tile_idx, PixelFuncRaw render_pixel);
};
