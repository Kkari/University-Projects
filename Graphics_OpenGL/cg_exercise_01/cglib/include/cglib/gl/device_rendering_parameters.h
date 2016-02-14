#pragma once

#include <cglib/core/parameters.h>

struct CTwBar;
/*
 * Raytracing parameters.
 *
 * Have a look at cglib/parameters.h to see which parameters are built-in already.
 * Feel free to add more parameters here. You can add them to the AntTweakBar gui,
 * aswell, to make interactive tweaking easier.
 */
class DeviceRenderingParameters : public Parameters
{
public:
	enum Renderers {
		RGBCUBE,
		GRAVITYFIELD,
		COLORMATCHING,
	};
	DeviceRenderingParameters() {}
	~DeviceRenderingParameters() {}

	/* restart means reinitialization for device rendering */
	virtual bool derived_change_requires_restart(Parameters const& old_) const override final;

	virtual void derived_gui_setup(CTwBar* main_bar) final;

	float near_clip = 0.05f, far_clip = 200.0f;

	float fov = 60.0f;
	Renderers current_renderer = RGBCUBE;

	bool enable_animation = true;
	
	struct Colors {
		struct RGBCube {
			bool showWireframe = false;
		} rgb_cube;

		struct GravityField {
			float mass0 = 0.050f;
			float mass1 = 0.020f;
			std::uint32_t tesselation = 32;
			bool showWireframe = false;
		} gravity_field;

		struct BlackBody {
			float temperature = 6500.f;
			std::uint32_t tesselation = 16;
			bool showWireframe = false;
		} blackbody;
	} colors;

};
