#include <cglib/core/parameters.h>

#include <AntTweakBar.h>

#include <iostream>
#include <sstream>
#include <thread>
#include <algorithm>

Parameters::Parameters()
{
	num_threads = std::max<std::uint32_t>(0, std::thread::hardware_concurrency());
#ifdef __APPLE__
	/* assume we have retina displays */
	screen_width = screen_height = 720;
#else
	screen_width  = image_width;
	screen_height = image_height;
#endif
}

// -----------------------------------------------------------------------------

bool Parameters::parse_command_line(int argc, char const** argv)
{
	for (int i = 1; i < argc; ++i)
	{
		std::string const arg = argv[i];

		if (arg == "--help" || arg == "-h")
		{
			std::cout
				<< "Usage: " << argv[0] << " [OPTION]...\n"
				<< "\n"
				<< "--create-images      Create assignment images.\n"
				<< "--noninteractive     Do not start in GUI mode.\n"
				<< "--stereo             Render in stereo mode.\n"
				<< "--eye-separation SEP Eye separation.\n"
				<< "--output FILE        The output file name when rendering in noninteractive mode.\n"
				<< "--width  N           The output image width.\n"
				<< "--height N           The output image height.\n"
				<< "--num-threads N      The number of threads to be used for rendering. Minimum 1.\n"
				<< "--tile-size N        The size of one work unit, in pixels.\n"
				<< "--fps N              The display rate.\n"
				<< "--help, -h           Display this information.\n"
				<< std::flush;
			return false;
		}

		// Options without arguments.
		if (arg == "--noninteractive")
		{
			interactive = false;
		}
		else if (arg == "--stereo")
		{
			stereo = true;
		}
		else if (arg == "--create-images")
		{
			create_images = true;
		}

		else
		{
			// Options with a parameter.
			++i;
			if (i >= argc)
			{
				std::cerr << "Option " << arg << " requires a parameter." << std::endl;
				return false;
			}

			std::istringstream is(argv[i]);
			bool success = true;

			if (arg == "--output")
			{
				is >> output_file_name;
			}


			else if (arg == "--width")
			{
				success = bool(is >> image_width);
			}

			else if (arg == "--height")
			{
				success = bool(is >> image_height);
			}

			else if (arg == "--num-threads")
			{
				success = bool(is >> num_threads);
				std::cout << "num_threads: " << num_threads << std::endl;
				num_threads = std::max<std::uint32_t>(1, num_threads);
				std::cout << "num_threads: " << num_threads << std::endl;
			}

			else if (arg == "--tile-size")
			{
				success = bool(is >> tile_size);
				tile_size = std::max<std::uint32_t>(1, tile_size);
			}

			else if (arg == "--fps")
			{
				success = bool(is >> fps);
				fps = std::max<std::uint32_t>(1, fps);
			}

			else if (arg == "--eye-separation")
			{
				success = bool(is >> eye_separation);
			}
			
			if (!success)
			{
				std::cerr << "Invalid parameter for option " << arg << ": '" << argv[i] << "'" << std::endl;
				return false;
			}
		}
	}

	return true;
}

// -----------------------------------------------------------------------------

bool Parameters::change_requires_restart(Parameters const& old) const
{
	const bool restart = false
		|| tile_size != old.tile_size
		|| derived_change_requires_restart(old)
		;
	return restart;
}

// -----------------------------------------------------------------------------

void Parameters::gui_setup()
{
	if(!interactive)
		return;
	const char *bar_name = "Settings";
	TwBar* bar = TwGetBarByName(bar_name);
	if(bar) {
		TwRemoveAllVars(bar);
	}
	else {
		bar = TwNewBar(bar_name);
	}

	derived_gui_setup(bar);

}

