#include <cglib/colors/physics.h>
#include <cglib/core/assert.h>

#include <cglib/colors/cmf.h>

#include <algorithm>
#include <cmath>

void blackbody_spectrum(std::vector<float>* spectrum, float temp)
{
	cg_assert(spectrum);

	static const float pi           = 3.14159265359f;  
	static const float hPlanck      = 6.62606957e-16f;  // In kg * nm² / s²
	static const float speedOfLight = 2.99792458e17f;   // In nm/s
	static const float kBoltzmann   = 1.3806488e-5f;    // In nm² * kg / (s² * K)

	float const c1 = 2.0f * pi * hPlanck * speedOfLight * speedOfLight;
	float const c2 = hPlanck * speedOfLight / (kBoltzmann * temp);

	spectrum->resize(cmf::wavelengths.size());
	float max_value = 0.f;
	for (std::size_t i = 0; i < cmf::wavelengths.size(); ++i)
	{
		float const lambda = cmf::wavelengths[i];
		spectrum->at(i) = c1 / (std::pow(lambda, 5.f) * (std::exp(c2 / lambda) - 1.f));
		max_value = std::max<float>(max_value, spectrum->at(i));
	}
	for (std::size_t i = 0; i < cmf::wavelengths.size(); ++i)
	{
		spectrum->at(i) /= max_value;
	}
}
