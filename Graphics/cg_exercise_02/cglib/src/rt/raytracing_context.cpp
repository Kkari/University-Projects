#include <cglib/rt/raytracing_context.h>

RaytracingContext *RaytracingContext::current_context = nullptr;

RaytracingContext::
RaytracingContext()
{
	cg_assert(current_context == nullptr);
	current_context = this;
}

RaytracingContext::
~RaytracingContext()
{
	cg_assert(current_context);
	current_context = nullptr;
}

RaytracingContext * RaytracingContext::
get_active()
{
	return current_context;
}
