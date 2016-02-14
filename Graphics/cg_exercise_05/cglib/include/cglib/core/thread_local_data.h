#pragma once

#include <random>

/*
 * Thread-local data.
 *
 * We render using multiple threads concurrently. Some data,
 * such as the random number generator, need to be thread-local
 * to avoid synchronization.
 *
 * You can add additional thread-local data if you would like to.
 */
struct ThreadLocalData
{
	// Random number generation.
	std::mt19937                          rng;
	std::uniform_real_distribution<float> dist;
	
	bool distributed_recursion = false;

	ThreadLocalData() {}

	virtual void initialize(int threadId) final
	{
		rng.seed(8890 + threadId);
	}

	inline float rand()
	{
		return dist(rng);
	}
};

