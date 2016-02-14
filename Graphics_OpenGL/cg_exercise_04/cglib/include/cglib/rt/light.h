#pragma once

#include <glm/glm.hpp>

class Light
{
public:
    Light(glm::vec3 const& position_, glm::vec3 const& emission_) :
        position(position_), emission(emission_)
    {
    }

	glm::vec3 getPosition() const { return position; }

	// evaluate the emission of the light in direction omega
	// (pointing away from the light).
	virtual glm::vec3 getEmission(glm::vec3 const& omega) const = 0;

protected:
    glm::vec3 position;
    glm::vec3 emission;
};

class PointLight : public Light
{
public:
    PointLight(glm::vec3 const& position_, glm::vec3 const& emission_) :
        Light(position_, emission_)
    {
    }

	// point lights emmit the same amount of light in all directions
	glm::vec3 getEmission(glm::vec3 const& /*omega*/) const {
		return emission;
	}
};

class SpotLight : public Light
{
public:
    SpotLight(glm::vec3 const& position_, glm::vec3 const& emission_, glm::vec3 const& direction_, float falloff_) :
        Light(position_, emission_),
		direction(direction_),
		falloff(falloff_)
    {
    }

	// emission characteristic of a spotlight
	glm::vec3 getEmission(glm::vec3 const& omega) const;

private:
	glm::vec3 direction;
	float falloff;
};

class AreaLight
{
public:
    AreaLight(glm::vec3 const& position_, glm::vec3 const& tangent_, glm::vec3 const& bitangent_, glm::vec3 const& emission_) :
        position(position_), tangent(tangent_), bitangent(bitangent_), emission(emission_)
    {
    }
	
    glm::vec3 position;
	glm::vec3 tangent;
	glm::vec3 bitangent;
    glm::vec3 emission;
};
