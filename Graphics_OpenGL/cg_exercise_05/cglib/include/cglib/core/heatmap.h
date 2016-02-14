#pragma once

#include <glm/glm.hpp>

inline glm::vec3
heatmap(float val)
{
	val = glm::clamp(val * 4.0f, 0.0f, 3.99999999999f);
	int idx = int(val);
	float interpol = glm::fract(val);

	const static glm::vec3 colors[5] = {
		glm::vec3(0, 0, 1),
		glm::vec3(0, 1, 1),
		glm::vec3(0, 1, 0),
		glm::vec3(1, 1, 0),
		glm::vec3(1, 0, 0)
	};

	return glm::mix(colors[idx], colors[idx + 1], interpol);
}
