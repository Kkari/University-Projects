#include <cglib/rt/host_render.h>
#include <cglib/rt/render_data.h>
#include <cglib/core/heatmap.h>
#include <cglib/rt/ray.h>
#include <cglib/rt/renderer.h>
#include <cglib/rt/bvh.h>

int HostRender::run(RaytracingContext& context, 
			   PixelFunc const& render_pixel, 
			   int kill_timeout_seconds,
			   std::function<void()> const& render_overlay)
{
	auto render_pixel_wrapper = [&](int x, int y, RaytracingContext const &ctx, ThreadLocalData *tld)
		-> glm::vec3
	{
		RenderData data(context, tld);

		switch(context.params.render_mode) {

		case RaytracingParameters::RECURSIVE:
			if (context.params.stereo)
			{
				data.camera_mode = Camera::StereoLeft;
				auto const left = render_pixel(x, y, ctx, data);
				data.camera_mode = Camera::StereoRight;
				auto const right = render_pixel(x, y, ctx, data);
				return combine_stereo(left, right);
			}
			else
			{
				return render_pixel(x, y, ctx, data);
			}

		case RaytracingParameters::DESATURATE:
			if (context.params.stereo)
			{
				data.camera_mode = Camera::StereoLeft;
				auto const left = render_pixel(x, y, ctx, data);
				data.camera_mode = Camera::StereoRight;
				auto const right = render_pixel(x, y, ctx, data);
				return combine_stereo(desaturate(left), desaturate(right));
			}
			else
			{
				return desaturate(render_pixel(x, y, ctx, data));
			}

		case RaytracingParameters::NUM_RAYS:
			render_pixel(x, y, ctx, data);
			return heatmap(float(data.num_cast_rays - 1) / 64.0f);
		case RaytracingParameters::NORMAL:
			render_pixel(x, y, ctx, data);
			if (context.params.normal_mapping)
				return glm::normalize(data.isect.shading_normal) * 0.5f + glm::vec3(0.5f);
			else
				return glm::normalize(data.isect.normal) * 0.5f + glm::vec3(0.5f);
		case RaytracingParameters::BVH_TIME:
		case RaytracingParameters::TIME: {
			Timer timer;
			timer.start();
			if(context.params.render_mode == RaytracingParameters::TIME) {
				auto const color = render_pixel(x, y, ctx, data);
				(void) color;
			}
			else {
				Ray ray = createPrimaryRay(data, float(x) + 0.5f, float(y) + 0.5f);
				for(auto& o: context.scene->objects) {
					BVH *bvh = dynamic_cast<BVH *>(o.get());
					if(bvh) {
						bvh->intersect(ray, nullptr);
					}
				}
			}
			timer.stop();
			return heatmap(static_cast<float>(timer.getElapsedTimeInMilliSec()) * context.params.scale_render_time);
		}
		case RaytracingParameters::DUDV: {
			auto const color = render_pixel(x, y, ctx, data);
			(void) color;
			if(!data.isect.isValid())
				return glm::vec3(0.0);
			return heatmap(std::log(1.0f + 5.0f * glm::length(data.isect.dudv)));
		}
		case RaytracingParameters::AABB_INTERSECT_COUNT: {
        	Ray ray = createPrimaryRay(data, float(x) + 0.5f, float(y) + 0.5f);
			glm::vec3 accum(0.0f);
			for(auto& o: context.scene->objects) {
				auto *bvh = dynamic_cast<BVH *>(o.get());
				if(bvh) {
					accum += bvh->intersect_count(ray, 0, 0) * 0.02f;
				}
			}
			return accum;
		}
		default: /* should never happen */
			return glm::vec3(1, 0, 1);
		}
	};

	if (context.params.interactive)
	{
		return run_interactive(context, render_pixel_wrapper, render_overlay);
	}
	else
	{
		return run_noninteractive(context, render_pixel_wrapper, 
			kill_timeout_seconds);
	}
}

// -----------------------------------------------------------------------------

void HostRender::generate_tile_idx(int num_tiles_x, int num_tiles_y, std::vector<glm::ivec2>* tile_idx)
{
	/* Generate tile indices in the order of a spiral that starts in the center of the image.
	 * This ensures that we will be able to see updates in the important region of the
	 * image quickly.
	 */
	int const num_tiles = num_tiles_x * num_tiles_y;

	tile_idx->resize(num_tiles);
	{
		static glm::ivec2 const dir[] = {
			glm::uvec2(1, 0),
			glm::uvec2(0, 1),
			glm::uvec2(-1, 0),
			glm::uvec2(0, -1)
		};

		glm::ivec2 current_idx(-1, 0);
		for (int i = 0, step = 0; i < num_tiles; ++step)
		{
			glm::ivec2 const d = dir[step % 4];
			int const size = (step % 2 == 0) ? num_tiles_x : num_tiles_y;
			int const num_step_tiles = size - (step+1) / 2;

			for (int j = 0; j < num_step_tiles && i < num_tiles; ++j, ++i)
			{
				cg_assert(i < num_tiles);
				current_idx += d;
				cg_assert(current_idx[0] < num_tiles_x);
				cg_assert(current_idx[1] < num_tiles_y);
                cg_assert(num_tiles-1-i >= 0);
				(*tile_idx)[num_tiles-1-i] = current_idx;
			}
		}
	}
}

// -----------------------------------------------------------------------------

int HostRender::run_noninteractive(RaytracingContext& context, 
	PixelFuncRaw const& render_pixel, int kill_timeout_seconds)
{
	Image      frame_buffer(context.params.image_width, context.params.image_height);
	ThreadPool thread_pool(context.params.num_threads);
	std::vector<glm::ivec2> tile_idx;

    Timer timer;
    timer.start();
	context.scene->refresh_scene(context.params);
	launch(&frame_buffer, thread_pool, &context, &tile_idx, render_pixel);

	if (kill_timeout_seconds > 0)
	{
		if (thread_pool.kill_at_timeout(kill_timeout_seconds))
		{
			cg_assert(!bool("Process ran into timeout - is there an infinite "
						    "loop?"));
		}
	}
	else
	{
		thread_pool.wait();
	}
	thread_pool.poll_exceptions();
    timer.stop();
    std::cout << "Rendering time: " << timer.getElapsedTimeInMilliSec() << "ms" << std::endl;
	frame_buffer.saveTGA(context.params.output_file_name.c_str(), 2.2f);

	return 0;
}

// -----------------------------------------------------------------------------

int HostRender::run_interactive(RaytracingContext& context, PixelFuncRaw const& render_pixel,
	std::function<void()> const& render_overlay)
{
	Image      frame_buffer(context.params.image_width, context.params.image_height);
	ThreadPool thread_pool(context.params.num_threads);
	std::vector<glm::ivec2> tile_idx;

	if (!GUI::init_host(context.params))
	{
		return 1;
	}

	if(context.scene)
		context.scene->set_active_camera();
    
	// Launch first render.
	launch(&frame_buffer, thread_pool, &context, &tile_idx, render_pixel);

	auto time_last_frame = std::chrono::high_resolution_clock::now();

	RaytracingParameters oldParams = context.params;
	while (GUI::keep_running())
	{
		GUI::poll_events();
		thread_pool.poll_exceptions();

		// Restart rendering if parameters have changed.
		auto cam = Camera::get_active();
		if ((cam && cam->requires_restart())
		|| context.params.change_requires_restart(oldParams))
		{
			thread_pool.terminate();
			if (oldParams.scene != context.params.scene) {
				// reload scene
				switch (context.params.scene) {
					case RaytracingParameters::MONKEY: context.scene = context.scenes["monkey"]; break;
					case RaytracingParameters::TRIANGLES: context.scene = context.scenes["triangles"]; break;
					case RaytracingParameters::SPONZA:
						if (!context.scenes.count("sponza")) {
							context.scenes.insert({"sponza", std::make_shared<SponzaScene>(context.params)});
						}
						context.scene = context.scenes["sponza"]; 
						break;
				default:
					cg_assert(!"should not happen");
				}
				cg_assert(context.scene);
				context.scene->set_active_camera();
			}
			if(context.params.spp < oldParams.spp) {
				while(int(sqrtf(static_cast<float>(context.params.spp))) * int(sqrtf(static_cast<float>(context.params.spp))) != context.params.spp
						&& context.params.spp >= 1) {
					context.params.spp--;
				}
			}
			else {
				while(int(sqrtf(static_cast<float>(context.params.spp))) * int(sqrtf(static_cast<float>(context.params.spp))) != context.params.spp) {
					context.params.spp++;
				}
			}
			context.scene->refresh_scene(context.params);
			oldParams = context.params;
			launch(&frame_buffer, thread_pool, &context, &tile_idx, render_pixel);
		}

		// Update the texture displayed online in regular intervals so that
		// we don't waste many cycles uploading all the time.
		auto const now = std::chrono::high_resolution_clock::now();
		float const mspf = 1000.f / static_cast<float>(context.params.fps);
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now-time_last_frame).count() > mspf)
		{
			GUI::display_host(frame_buffer, render_overlay);
		}
	}

	GUI::cleanup();

	return 0;
}

// -----------------------------------------------------------------------------

void HostRender::launch(Image* fb, 
		         ThreadPool& thread_pool, 
				 RaytracingContext const* context, 
				 std::vector<glm::ivec2>* tile_idx,
				 PixelFuncRaw render_pixel)
{
    if (!thread_pool.enough_progress())
    {
        //return;
    }

	// Clean up.
	thread_pool.terminate();
	fb->clear(glm::vec4(0.f));

	// Compute number of tiles (work units).
	int const width  = fb->getWidth();
	int const height = fb->getHeight();
	int const tile_size   = context->params.tile_size;
	int const num_tiles_x = static_cast<int>(std::ceil(float(width) / float(tile_size)));
	int const num_tiles_y = static_cast<int>(std::ceil(float(height) / float(tile_size)));
	int const num_tiles   = num_tiles_x * num_tiles_y;

	// New tile indices.
	generate_tile_idx(num_tiles_x, num_tiles_y, tile_idx);

	// Launch threads.
	thread_pool.run<ThreadLocalData>(num_tiles, 
		// The actual kernel.
		[=](int tile, ThreadLocalData* tld, std::atomic<bool>& terminate)
		{
			glm::ivec2 const idx = (*tile_idx)[tile];
			int const baseX = std::max<int>(idx[0] * tile_size, 0);
			int const endX  = std::min<int>(baseX + tile_size, width);

			int const baseY = std::max<int>(idx[1] * tile_size, 0);
			int const endY  = std::min<int>(baseY + tile_size, height);

            Image img(endX-baseX, endY-baseY);
			for (int y = baseY; y < endY; y++) 
			{
				for (int x = baseX; x < endX; x++) 
				{
					if (terminate.load())
						return;

					glm::vec3 const color = render_pixel(x, y, *context, dynamic_cast<ThreadLocalData*>(tld));
					img.setPixel(x-baseX, y-baseY, glm::vec4(color, 1.f));
				}
			}

            std::lock_guard<std::mutex> lock(mutex);
			for (int y = baseY; y < endY; y++) 
			{
				for (int x = baseX; x < endX; x++) 
				{
                    fb->setPixel(x, y, img.getPixel(x-baseX, y-baseY));
                }
            }

		}
	);
}
