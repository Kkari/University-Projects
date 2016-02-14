#include <cglib/rt/texture.h>

#include <cglib/core/image.h>
#include <cglib/core/glmstream.h>
#include <cglib/core/assert.h>

#include <algorithm>

ImageTexture::ImageTexture(
    std::string const& filename,
    TextureFilterMode filter_mode_,
    TextureWrapMode wrap_mode_,
    float gamma_) :
    Texture(),
    filter_mode(filter_mode_),
    wrap_mode(wrap_mode_)
{
    mip_levels.emplace_back(new Image());
    mip_levels.back()->load(filename.c_str(), gamma_);
	create_mipmap();
}

ImageTexture::ImageTexture(
    Image const& image,
    TextureFilterMode filter_mode_,
    TextureWrapMode wrap_mode_) :
    Texture(),
    filter_mode(filter_mode_),
    wrap_mode(wrap_mode_)
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

void ImageTexture::
create_mipmap()
{
	/* iteratively downsample until only a 1x1 image is left */
	int size_x = mip_levels[0]->getWidth();
	int size_y = mip_levels[0]->getHeight();

	cg_assert("must be power of two" && !(size_x & (size_x - 1)));
	cg_assert("must be power of two" && !(size_y & (size_y - 1)));
	
	for (int level = 0; size_x > 1 || size_y > 1; level++)
	{
		int const cx = size_x > 1 ? 2 : 1;
		int const cy = size_y > 1 ? 2 : 1;
		size_x = std::max(1, size_x/2);
		size_y = std::max(1, size_y/2);
		mip_levels.emplace_back(new Image(size_x, size_y));
		for (int x = 0; x < size_x; x++) {
			for (int y = 0; y < size_y; y++) {
				glm::vec4 mean(0.f);
				for (int xx = 0; xx < cx; xx++) {
					for (int yy = 0; yy < cy; yy++) {
						mean += mip_levels[level]->getPixel(2*x+xx, 2*y+yy);
					}
				}
				mip_levels[level+1]->setPixel(x, y, mean/float(cx*cy));
			}
		}
	}
}

glm::vec4 ImageTexture::
evaluate_nearest(int level, glm::vec2 const& uv) const
{
	cg_assert(level >= 0 && level < static_cast<int>(mip_levels.size()));
	cg_assert(mip_levels[level]);
	int const width = mip_levels[level]->getWidth();
	int const height = mip_levels[level]->getHeight();
	int const s = (int)std::floor(uv[0]*width);
	int const t = (int)std::floor(uv[1]*height);
	return get_texel(level, s, t);
}

glm::vec4 ImageTexture::
evaluate_bilinear(int level, glm::vec2 const& uv) const
{
	cg_assert(level >= 0 && level < static_cast<int>(mip_levels.size()));
	cg_assert(mip_levels[level]);
	int const width = mip_levels[level]->getWidth();
	int const height = mip_levels[level]->getHeight();
	float fs = uv[0]*width+0.5f;
	float ft = uv[1]*height+0.5f;
	float const ffs = std::floor(fs);
	float const fft = std::floor(ft);
	float const ws = fs - ffs;
	float const wt = ft - fft;

	return (1.f-ws) * (1.f-wt) * get_texel(level, ffs-1, fft-1) + 
		   (1.f-ws) * (    wt) * get_texel(level, ffs-1, fft) + 
		   (    ws) * (1.f-wt) * get_texel(level, ffs,   fft-1) + 
		   (    ws) * (    wt) * get_texel(level, ffs,   fft); 
}

glm::vec4 ImageTexture::
evaluate_trilinear(glm::vec2 const& uv, glm::vec2 const& dudv) const
{
	const float footprint_size = std::max(1.f, std::max(
		dudv[0]*mip_levels[0]->getWidth(), dudv[1]*mip_levels[0]->getHeight()));

	const float level = std::log2(footprint_size);
	const float alpha = glm::fract(level);
	const int lower = std::min<int>(std::max<int>(0, static_cast<int>(std::floor(level))), mip_levels.size()-1);
	const int upper = std::min<int>(std::max<int>(0, static_cast<int>(std::ceil(level))), mip_levels.size()-1);

	// visualization of mipmap level
	//return       alpha  * glm::vec3(float(upper)/(mip_levels.size()-1)) 
	//    + (1.f - alpha) * glm::vec3(float(lower)/(mip_levels.size()-1));
	
	return      alpha * evaluate_bilinear(upper, uv) 
		+ (1.f-alpha) * evaluate_bilinear(lower, uv);
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

int ImageTexture::
wrap_clamp(int val, int size)
{
	return std::max(0, std::min(size-1, val));
}

int ImageTexture::
wrap_repeat(int val, int size)
{
	return (val % size + size) % size;
}

