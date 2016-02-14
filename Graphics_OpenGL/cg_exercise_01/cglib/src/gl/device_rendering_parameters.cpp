#include <cglib/gl/device_rendering_parameters.h>
#include <cglib/gl/device_rendering_context.h>
#include <cglib/core/assert.h>

#include <AntTweakBar.h>
#include <sstream>

#include <cglib/colors/physics.h>

#define LENGTH(a) ((sizeof(a))/sizeof((a)[0]))

static TwEnumVal renderer_enum[] = {
	{ DeviceRenderingParameters::RGBCUBE,           "RGB Cube"    },
	{ DeviceRenderingParameters::GRAVITYFIELD,      "GravityField"},
	{ DeviceRenderingParameters::COLORMATCHING, 	"ColorMatching"},
};

bool DeviceRenderingParameters::
derived_change_requires_restart(Parameters const& old_) const
{
	auto const* old = dynamic_cast<DeviceRenderingParameters const*>(&old_);
	cg_assert(old); // this would be a serious implementation error

#define CHECK_PARAM(a) || ((a) != old->a)
	const bool restart = false
		CHECK_PARAM(screen_width)
		CHECK_PARAM(screen_height)
		CHECK_PARAM(fov)
		CHECK_PARAM(near_clip)
		CHECK_PARAM(far_clip)
		CHECK_PARAM(current_renderer)
		CHECK_PARAM(enable_animation)

		CHECK_PARAM(colors.rgb_cube.showWireframe)
		CHECK_PARAM(colors.gravity_field.mass0)
		CHECK_PARAM(colors.gravity_field.mass1)
		CHECK_PARAM(colors.gravity_field.tesselation)
		CHECK_PARAM(colors.gravity_field.showWireframe)
		CHECK_PARAM(colors.blackbody.temperature)
		CHECK_PARAM(colors.blackbody.tesselation)
		CHECK_PARAM(colors.blackbody.showWireframe)
		;
#undef CHECK_PARAM

	return restart;
}

void DeviceRenderingParameters::
derived_gui_setup(TwBar *bar)
{
	cg_assert(bar);
	TwType render_mode_type = TwDefineEnum("Renderer", renderer_enum, LENGTH(renderer_enum));

	TwAddVarRW(bar, "current_renderer", render_mode_type, &current_renderer, "label='Renderer' group='Rendering Settings'");

	if(0
	) {
		TwAddVarRW(bar, "animation", TW_TYPE_BOOLCPP, &enable_animation, "label='Animation' group='Rendering Settings'");
	}

	if(current_renderer == RGBCUBE) {
		TwAddVarRW(bar, "cube wireframe", TW_TYPE_BOOL8, &colors.rgb_cube.showWireframe, "label='wireframe' group='RGB Cube'");
	}
	if(current_renderer == GRAVITYFIELD) {
		TwAddVarRW(bar, "gridWireframe", TW_TYPE_BOOL8, &colors.gravity_field.showWireframe, "label='wireframe' group='HSV Heightmap'");
		TwAddVarRW(bar, "gridTesselation", TW_TYPE_UINT32, &colors.gravity_field.tesselation, "label='Tesselation' group='HSV Heightmap' min=1");
		TwAddVarRW(bar, "mass 0", TW_TYPE_FLOAT, &colors.gravity_field.mass0, "label='mass 0' group='HSV Heightmap' min=0.001 step=0.001");
		TwAddVarRW(bar, "mass 1", TW_TYPE_FLOAT, &colors.gravity_field.mass1, "label='mass 1' group='HSV Heightmap' min=0.001 step=0.001");
	}
	if (current_renderer == COLORMATCHING) {
		std::ostringstream options;
		options << "label='Current Temperature' group='Blackbody' step=5 min=" << BLACKBODY_TMIN << " max=" << BLACKBODY_TMAX;
		TwAddVarRW(bar, "blackBodyTemperature", TW_TYPE_FLOAT, &colors.blackbody.temperature, options.str().c_str());
		TwAddVarRW(bar, "blackBodyTesselation", TW_TYPE_UINT32, &colors.blackbody.tesselation, "label='Tesselation' group='Blackbody' min=1");
		TwAddVarRW(bar, "BB wireframe", TW_TYPE_BOOL8, &colors.blackbody.showWireframe, "label='wireframe' group='Blackbody'");

	}

}
