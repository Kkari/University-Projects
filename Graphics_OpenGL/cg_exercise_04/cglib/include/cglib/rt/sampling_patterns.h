#pragma once

#include <vector>
#include <glm/glm.hpp>

struct ThreadLocalData;

void
generate_stratified_samples(
		std::vector<glm::vec2> *generated_samples,
		int grid_x,
		int grid_y,
		ThreadLocalData *tld);

void
generate_random_samples(
		std::vector<glm::vec2> *generated_samples,
		int grid_x,
		int grid_y,
		ThreadLocalData *tld);
