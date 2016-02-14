#include <cglib/rt/texture.h>

#include <cglib/core/image.h>
#include <cglib/core/glmstream.h>
#include <cglib/core/assert.h>

#include <algorithm>

ImageTexture::ImageTexture(
    std::string const& filename,
    TextureFilterMode filter_mode,
    TextureWrapMode wrap_mode,
    float gamma) :
    Texture(),
    filter_mode(filter_mode),
    wrap_mode(wrap_mode)
{
    mip_levels.emplace_back(new Image());
    mip_levels.back()->load(filename.c_str(), gamma);
	create_mipmap();
}

ImageTexture::ImageTexture(
    Image const& image,
    TextureFilterMode filter_mode,
    TextureWrapMode wrap_mode) :
    Texture(),
    filter_mode(filter_mode),
    wrap_mode(wrap_mode)
{
    mip_levels.emplace_back(new Image(image.getWidth(), image.getHeight()));
	for (int y = 0; y < image.getHeight(); ++y) {
		for (int x = 0; x < image.getWidth(); ++x) {
			mip_levels.back()->setPixel(x, y, image.getPixel(x, y));

		}
	}
	create_mipmap();
}

glm::vec4 ImageTexture::
evaluate(glm::vec2 const& uv, glm::vec2 const& dudv) const
{
	switch(filter_mode) {
		case NEAREST:   return evaluate_nearest(0, uv);
		case BILINEAR:  return evaluate_bilinear(0, uv);
		case DEBUG_MIP:
		case TRILINEAR: return evaluate_trilinear(uv, dudv);
		default:        return glm::vec4(0.f);
	}
	return glm::vec4(0.f);
}

glm::vec4 ImageTexture::
get_texel(int level, int x, int y) const
{
	static glm::vec4 mip_level_debug_colors[] = {
		{ 1, 0, 0, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 0, 1, 0 },
		{ 1, 1, 0, 0 },
		{ 1, 0, 1, 0 },
		{ 0, 1, 1, 0 },
	};
	cg_assert(level >= 0 && level < int(mip_levels.size()));
	cg_assert(mip_levels.at(level)->getWidth() > 0);
	cg_assert(mip_levels.at(level)->getHeight() > 0);

	if(filter_mode == DEBUG_MIP) {
		int l = level % (sizeof(mip_level_debug_colors)
				/ sizeof(mip_level_debug_colors[0]));
		return mip_level_debug_colors[l];
	}


	switch (wrap_mode)
	{
		case REPEAT:
			x = wrap_repeat(x, mip_levels[level]->getWidth());
			y = wrap_repeat(y, mip_levels[level]->getHeight());
			break;

		case CLAMP:
			x = wrap_clamp(x, mip_levels[level]->getWidth());
			y = wrap_clamp(y, mip_levels[level]->getHeight());
			break;

		case ZERO:
			if (x < 0 || x >= mip_levels[level]->getWidth()
			 || y < 0 || y >= mip_levels[level]->getHeight())
			{
				return glm::vec4(0);
			}
			break;
		default:
			cg_assert(!"Invalid pixel wrap mode.");
			return glm::vec4(0);
	}

	cg_assert(x >= 0 && x < mip_levels[level]->getWidth());
	cg_assert(y >= 0 && y < mip_levels[level]->getHeight());

	return mip_levels[level]->getPixel(x, y);
}

void ImageTexture::set_texel(int level, int x, int y, glm::vec4 const& value)
{
	cg_assert(level >= 0 && level < int(mip_levels.size()));
	cg_assert(mip_levels.at(level)->getWidth() > 0);
	cg_assert(mip_levels.at(level)->getHeight() > 0);
	cg_assert(x >= 0 && x < mip_levels.at(level)->getWidth());
	cg_assert(y >= 0 && y < mip_levels.at(level)->getHeight());
	mip_levels[level]->setPixel(x, y, value);
}

