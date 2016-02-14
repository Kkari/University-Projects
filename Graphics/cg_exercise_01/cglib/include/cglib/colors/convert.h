#ifndef CONVERT_H_
#define CONVERT_H_

#include <glm/vec3.hpp>

/*
 * Some helper functions that convert between
 * the RGB, xyY and HSV color spaces.
 */
namespace convert {
	using namespace glm;

	inline vec3 rgb_to_hsv(vec3 const& rgb) 
	{
		vec3 hsv(0.f, 0.f, 0.f);
		int maxChan = 0;
		int minChan = 0;
		if (rgb[1] > rgb[maxChan]) maxChan = 1;
		if (rgb[2] > rgb[maxChan]) maxChan = 2;
		if (rgb[1] < rgb[minChan]) minChan = 1;
		if (rgb[2] < rgb[minChan]) minChan = 2;
		const float range = rgb[maxChan]-rgb[minChan];

		// Hue.
		if (maxChan != minChan) {
			if (maxChan == 0) hsv.x = 60.f * (rgb.y-rgb.z) / range;
			if (maxChan == 1) hsv.x = 60.f * (2.f + (rgb.z-rgb.x) / range);
			if (maxChan == 2) hsv.x = 60.f * (4.f + (rgb.x-rgb.y) / range);
		}
		if (hsv.x < 0.f)  hsv.x += 360.f;
		hsv.x /= 360.f;

		// Saturation.
		if (rgb[maxChan] < 1e-4) hsv.y = 0.f;
		else hsv.y = range / rgb[maxChan];

		// Value.
		hsv.z = rgb[maxChan];
		return hsv;
	}

	inline vec3 hsv_to_rgb(vec3 const& hsv) 
	{
		const float interval = hsv.x * 6.f; // assuming hue is in [0, 1].
		const int base = static_cast<int>(std::floor(interval));
		const float offset = interval - static_cast<float>(base);
		const float p = hsv.z * (1.f - hsv.y);
		const float q = hsv.z * (1.f - hsv.y * offset);
		const float t = hsv.z * (1.f - hsv.y * (1.f - offset));
		switch (base) {
			case 1: return vec3(q, hsv.z, p);
			case 2: return vec3(p, hsv.z, t);
			case 3: return vec3(p, q, hsv.z);
			case 4: return vec3(t, p, hsv.z);
			case 5: return vec3(hsv.z, p, q);
			default: return vec3(hsv.z, t, p);
		}
	}

	inline vec3 rgb_to_xyz(vec3 const& rgb) 
	{
		const mat3 M ( // Column-major!
			0.4124564, 0.2126729, 0.0193339,
			0.3575761, 0.7151522, 0.1191920,
			0.1804375, 0.0721750, 0.9503041
		);
		return M * rgb;
	}

	inline vec3 xyz_to_rgb(vec3 const& xyz) 
	{
		const mat3 M ( // Column-major!
			3.2404542, -0.9692660, 0.0556434,
			-1.5371385, 1.8760108, -0.2040259,
			-0.4985314, 0.0415560, 1.0572252
		);
		return M * xyz;
	}

	inline vec3 xyz_to_xyy(vec3 const& xyz) 
	{
		const float sum = xyz.x + xyz.y + xyz.z;

		// For Y = 0, we need to return the chroma of the
		// white point, or horrible xyz shifts will ensue.
		if (xyz.y < 1e-4) {
			const vec3 whitePoint = rgb_to_xyz(vec3(1.f));
			const float whiteSum = whitePoint.x + whitePoint.y + whitePoint.z;
			const vec2 chromaWhite(whitePoint.x / whiteSum, whitePoint.y / whiteSum);
			return vec3(chromaWhite.x, chromaWhite.y, 0.f);
		}
		return vec3(xyz.x / sum, xyz.y / sum, xyz.y);
	}

	inline vec3 xyy_to_xyz(vec3 const& xyy) 
	{
		if (xyy.z < 1e-4) return vec3(0.f);
		const float sum = xyy.z / xyy.y;

		return vec3(xyy.x * sum,
				 xyy.z,
				 (1.f - xyy.x - xyy.y) * sum);
	}

	inline vec3 rgb_to_xyy(vec3 const& rgb) 
	{
		return xyz_to_xyy(rgb_to_xyz(rgb));
	}

	inline vec3 xyy_to_rgb(vec3 const& xyy) 
	{
		return xyz_to_rgb(xyy_to_xyz(xyy));
	}
}

#endif
