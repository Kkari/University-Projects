#pragma once

inline float luminance(glm::vec3 const& color)
{
	return glm::dot(glm::vec3(0.299f, 0.587f, 0.114f), color);
}

// -----------------------------------------------------------------------------

inline glm::vec3 desaturate(glm::vec3 const& color)
{
	return glm::vec3(luminance(color));
}

// -----------------------------------------------------------------------------

inline glm::vec3 combine_stereo(glm::vec3 const& left, glm::vec3 const& right)
{
	/*
	 * Green - Red rendering
	 * This can only reproduce monochrome images.
	 */
	// glm::vec3 const filter[] = 
	// {
	// 	glm::vec3(0.f, 1.f, 1.f), // Left channel has all red filtered out - will be black in right eye.
	// 	glm::vec3(1.f, 0.f, 1.f)  // Right channel has all green filtered out -  will be black in left eye.
	// };

	/* 
	 * Cyan - Red rendering 
	 */
	glm::vec3 const filter[] = 
	{
		glm::vec3(0.f, 1.f, 1.f), 
		glm::vec3(1.f, 0.f, 0.f)  
	};

	glm::vec3 const* src[] = { &left, &right };

	glm::vec3 color(0.f);
	for (int i = 0; i < 2; ++i)
	{
		glm::vec3 const filtered  = filter[i] * *(src[i]);
		float const lum           = luminance(*(src[i]));
		float const filtered_lum  = luminance(filtered);
		if (filtered_lum > 0.f)
		{
			color += filtered * lum / filtered_lum;
		}
	}

	return color;
}

// -----------------------------------------------------------------------------


