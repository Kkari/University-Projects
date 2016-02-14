#pragma once

#include <cglib/core/camera.h>

#include <cstdint>
#include <string>

struct CTwBar;

/*
 * Derive from this class to set your own parameters.
 */
class Parameters
{
public:
	//
	// Standard parameters that we always want to have.
	//

	// Should we create assignment images
	bool create_images = false;
	
	// Should we render in stereo mode?
	bool stereo = false;

	// Eye separation.
	float eye_separation = 0.1f;

	// The number of threads to be used for rendering. Defaults to number of hardware threads -1.
	std::uint32_t  num_threads;

	// The size of the image to be rendered.
	std::uint32_t image_width  = 512;
	std::uint32_t image_height = 512;

	// The size of the window
	std::uint32_t screen_width;
	std::uint32_t screen_height;

	// Output filename (used for noninteractive renders).
	std::string output_file_name = "output.tga";

	// The size of a render tile.
	std::uint32_t tile_size = 32;

	// In gui mode, display with this many frames per second.
	std::uint32_t fps = 60;
	
	// Run in interactive mode?
	bool interactive = true;

	float exposure = 1.0f;
	float gamma = 2.2f;

// -------------------------------------------------------------------------

public:
	Parameters();
	virtual ~Parameters() {}

	// Parse the command line for options we support.
	bool parse_command_line(int argc, char const** argv);

	// Does the change in parameters require a render restart?
	bool change_requires_restart(Parameters const& old) const;

	// Add parameters to the AntTweakBar gui.
	void gui_setup();

protected:
	// Implement the following to handle your own parameters.
	virtual bool derived_change_requires_restart(Parameters const& old) const { return false; }

	// Implement the following to handle your own parameters.
	virtual void derived_gui_setup(CTwBar *main_bar) {}
};

