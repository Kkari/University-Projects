#ifndef COLOR_MATCHING_H_
#define COLOR_MATCHING_H_

#include <vector>
#include <glm/glm.hpp>

/*
 * This struct defines the the X, Y and Z
 * color matching functions and the according
 * wavelengths.
 *
 * For example, the X color matching function
 * is specified for wavelengths
 *     wavelengths[0], ..., wavelengths[N] 
 * with the according spectral values 
 *     x[0], ..., x[N]
 */ 
struct cmf 
{
	static const std::vector<float> wavelengths;
	static const std::vector<float> x;
	static const std::vector<float> y;
	static const std::vector<float> z;
};

#endif

