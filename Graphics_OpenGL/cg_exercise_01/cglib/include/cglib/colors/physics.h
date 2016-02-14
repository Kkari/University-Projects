#pragma once

#include <vector>
#include <cmath>
#include <algorithm>

/*
 * The temperature range for our blackbody spectrum.
 */
static float const BLACKBODY_TMIN = 370;     // About 100°C.
static float const BLACKBODY_TMAX = 8000.f;  // Some arbitrary limit to facilitate color mapping.

/*
 * Generate the blackbody spectrum for the given temperature.
 * Note: In order to facilitate color mapping, these spectra are normalized
 * to maximum 1. This is technically wrong.
 */
void blackbody_spectrum(std::vector<float>* spectrum, float temp);

/*
 * Colormap the very high dynamic range blackbody spectrum.
 */
inline float colormap_luminance(float Y)
{
	return std::min<float>(Y, 1.f);
}

/*
 * Gravitational potential at a point mass m in distance d.
 */
inline float gravitational_potential(float m, float d)
{
	return -m / d;
}

