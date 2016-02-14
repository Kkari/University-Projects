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

class ImageTexture : public Texture
{
public:
    ImageTexture(
        std::string const& filename,
        TextureFilterMode filter_mode,
        TextureWrapMode wrap_mode,
        float gamma = 2.f);
    ImageTexture(
        Image const& img,
        TextureFilterMode filter_mode,
        TextureWrapMode wrap_mode);

	glm::vec4 evaluate(glm::vec2 const& uv, glm::vec2 const& dudv) const override;
    glm::vec4 evaluate_nearest(int level, glm::vec2 const& uv) const;
    glm::vec4 evaluate_bilinear(int level, glm::vec2 const& uv) const;
    glm::vec4 evaluate_trilinear(glm::vec2 const& uv, glm::vec2 const& dudv) const;

	static int wrap_repeat(int val, int size);
	static int wrap_clamp(int val, int size);
	glm::vec4 get_texel(int level, int x, int y) const;
	void set_texel(int level, int x, int y, glm::vec4 const& val);

	std::vector<std::shared_ptr<Image>> const& get_mip_levels() const {
		return mip_levels;
	}

private:
    void create_mipmap();
public:
    TextureFilterMode filter_mode;
    TextureWrapMode wrap_mode;
private:
    std::vector<std::shared_ptr<Image>> mip_levels; // the different mip map textures
};

typedef std::unordered_map<std::string, std::shared_ptr<ImageTexture>> TextureContainer;

