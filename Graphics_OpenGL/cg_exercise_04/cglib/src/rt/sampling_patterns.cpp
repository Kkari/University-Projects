#include <cglib/rt/sampling_patterns.h>
#include <glm/glm.hpp>
#include <cglib/core/thread_local_data.h>

void
generate_random_samples(
		std::vector<glm::vec2> *generated_samples,
		int grid_x,
		int grid_y,
		ThreadLocalData *tld)
{
	generated_samples->resize(grid_x * grid_y);

	for(int y = 0; y < grid_y; y++) {
		for(int x = 0; x < grid_x; x++) {
			glm::vec2 &s = (*generated_samples)[y * grid_x + x];
			s = (glm::vec2(tld->rand(), tld->rand()));
		}
	}
}

void
generate_stratified_samples(
		std::vector<glm::vec2> *generated_samples,
		int grid_x,
		int grid_y,
		ThreadLocalData *tld)
{
	generated_samples->resize(grid_x * grid_y);

	for(int y = 0; y < grid_y; y++) {
		for(int x = 0; x < grid_x; x++) {
			glm::vec2 &s = (*generated_samples)[y * grid_x + x];
			s = (glm::vec2(x, y) + glm::vec2(tld->rand(), tld->rand()))
				/ glm::vec2(grid_x, grid_y);
		}
	}
}
