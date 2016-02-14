#pragma once

#include <glm/glm.hpp>

#include <iostream>

// This is the default light implementation
// which represents a point light and emitts
// light equally in all directions
class Light
{
public:
    Light(glm::vec3 const& position_, glm::vec3 const& power_) :
        position(position_), power(power_)
    {
    }

	glm::vec3 getPosition() const { return position; }

	// returns a random unformly distributed point on the light source
	virtual glm::vec3 uniform_sample_point(float x0, float x1) const { return position; } 
	virtual float get_area() const { return 0.f; }
	
	// evaluate the emission of the light in direction omega
	// (pointing away from the light).
	virtual glm::vec3 getEmission(glm::vec3 const& omega) const { return power/(4.f*float(M_PI)); }
	virtual glm::vec3 getPower() const { return power; }

protected:
    glm::vec3 position;
    glm::vec3 power;
};

class AreaLight : public Light
{
public:
    AreaLight(
		glm::vec3 const& position_, 
		glm::vec3 const& tangent_, 
		glm::vec3 const& bitangent_,
		glm::vec3 const& power_) :
		Light(position_, power_),
		tangent(tangent_), 
		bitangent(bitangent_)
    {
		normal = glm::normalize(glm::cross(tangent, bitangent));
    }

	// returns a random unformly distributed point on the light source
	virtual glm::vec3 uniform_sample_point(float x0, float x1) const;

	virtual float get_area() const 
	{ 
		return glm::length(tangent)*glm::length(bitangent); 
	}
	virtual glm::vec3 getEmission(glm::vec3 const& omega) const { 
		return power * std::max(0.f, glm::dot(normal, omega)) / 
			(2.f*float(M_PI)*get_area()); 
	}
	
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 bitangent;
};
