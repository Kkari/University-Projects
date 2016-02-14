#pragma once

#include <cglib/rt/ray.h>

#include <glm/glm.hpp>

struct AABB
{
	glm::vec3 min = glm::vec3( FLT_MAX);
	glm::vec3 max = glm::vec3(-FLT_MAX);

	void extend(const glm::vec3 &v)
	{
		this->min = glm::min(this->min, v - glm::vec3(1e-4f));
		this->max = glm::max(this->max, v + glm::vec3(1e-4f));
	}

	bool
	intersect(const Ray &ray, float &t_min, float &t_max) const
	{
		glm::vec3 div = 1.0f / ray.direction;
		return intersect(ray, t_min, t_max, div);
	}

	bool
	intersect(const Ray &ray, float &t_min, float &t_max, const glm::vec3 div) const
	{
		//glm::vec3 div = 1.0f / ray.direction;
		glm::vec3 t_1 = (this->min - ray.origin) * div;
		glm::vec3 t_2 = (this->max - ray.origin) * div;

		glm::vec3 t_min2 = glm::min(t_1, t_2);
		glm::vec3 t_max2 = glm::max(t_1, t_2);

		t_min = glm::max(glm::max(t_min2.x, t_min2.y), glm::max(t_min2.z, t_min));
		t_max = glm::min(glm::min(t_max2.x, t_max2.y), glm::min(t_max2.z, t_max));

		return t_min <= t_max;
	}

	bool contains(AABB const& other) const
	{
		for (int i = 0; i < 3; ++i)
		{
			if ((other.min[i] < min[i])
			 || (other.max[i] > max[i]))
			{
				return false;
			}
		}
		return true;
	}

	bool is_empty() const
	{
		for (int i = 0; i < 3; ++i)
		{
			if (min[i] > max[i]) { return false; }
		}
		return true;
	}
};

