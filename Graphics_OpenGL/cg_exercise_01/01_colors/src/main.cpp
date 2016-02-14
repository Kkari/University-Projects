#include <cglib/colors/exercise.h>
#include <cglib/colors/convert.h>

#include <cglib/gl/renderer.h>
#include <cglib/gl/device_rendering_context.h>
#include <cglib/gl/device_render.h>
#include <cglib/core/glheaders.h>
#include <cglib/core/glmstream.h>


int
main(int argc, char const**argv)
{
	// This call will take care of launching a gui (or not, in case we do noninteractive rendering),
	// and also run the main rendering loop.
	//
	// In general, this will first call init(), and then repeatedly call render(), both
	// of which are defined above.
	//
	// If you would like to know more about how the gui works, please refer to cglib code.
    DeviceRenderingContext context;
    if (!context.params.parse_command_line(argc, argv)) {
        std::cerr << "invalid command line argument" << std::endl;
        return -1;
    }

    context.renderers.insert({DeviceRenderingParameters::RGBCUBE,         std::make_shared<RGBCubeRenderer>()});
    context.renderers.insert({DeviceRenderingParameters::GRAVITYFIELD,    std::make_shared<GravityFieldRenderer>()});
    context.renderers.insert({DeviceRenderingParameters::COLORMATCHING,    std::make_shared<ColorMatchingRenderer>()});

    //return DeviceRender::run(context, init, render, false);
    return DeviceRender::run(context, false);
}

// CG_REVISION 056a6d17a95c6120012a7c57a57427812405eac9
