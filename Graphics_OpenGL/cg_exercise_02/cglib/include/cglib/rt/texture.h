#pragma once

#include <math.h>

#include <glm/glm.hpp>

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>

class Image;

enum TextureFilterMode {NEAREST, BILINEAR, TRILINEAR, DEBUG_MIP};
enum TextureWrapMode {REPEAT, CLAMP, ZERO};

class Texture
{ 
public:
	virtual ~Texture() {}
	virtual glm::vec4 evaluate(glm::vec2 const& uv, glm::vec2 const& dudv = glm::vec2(0.f)) const = 0;
};

class ConstTexture : public Texture
{
public:
    ConstTexture(glm::vec3 const& value) :
        Texture(),
        value(value)
    {}

	glm::vec4 evaluate(glm::vec2 const& /*uv*/, glm::vec2 const& /*dudv*/) const override
    {
        return glm::vec4(value, 0.0f);
    }

private:
    glm::vec3 value;
};

