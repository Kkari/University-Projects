#pragma once

#include <glm/glm.hpp>

class Light
{
public:
    Light(glm::vec3 const& position, glm::vec3 const& emission) :
        position(position), emission(emission)
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
    PointLight(glm::vec3 const& position, glm::vec3 const& emission) :
        Light(position, emission)
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
    SpotLight(glm::vec3 const& position, glm::vec3 const& emission, glm::vec3 const& direction, float falloff) :
        Light(position, emission),
		direction(direction),
		falloff(falloff)
    {
    }

	// emission characteristic of a spotlight
	// TODO: implement this in exercise.cpp
	glm::vec3 getEmission(glm::vec3 const& omega) const;

private:
	glm::vec3 direction;
	float falloff;
};

class AreaLight
{
public:
    AreaLight(glm::vec3 const& position, glm::vec3 const& tangent, glm::vec3 const& bitangent, glm::vec3 const& emission) :
        position(position), tangent(tangent), bitangent(bitangent), emission(emission)
    {
    }
	
    glm::vec3 position;
	glm::vec3 tangent;
	glm::vec3 bitangent;
    glm::vec3 emission;
};
