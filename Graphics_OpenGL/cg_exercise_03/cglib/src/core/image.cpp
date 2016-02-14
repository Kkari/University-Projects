#include <cglib/core/image.h>
#include <cglib/core/stb_image.h>
#include <cglib/core/assert.h>

#include <cstdlib>
#include <cstdint>
#include <cmath>
#include <cstring>
#include <iostream>
#include <fstream>
#include <algorithm>

#ifdef __WIN32__
	#pragma pack(push,1)
#endif
	struct TgaHeader
	{
		TgaHeader() {
			std::memset(color_map_spec, 0, sizeof color_map_spec);
			std::memset(origin, 0, sizeof origin);
			std::memset(size, 0, sizeof size);
		}

		std::uint8_t  image_id_length = 0;
		std::uint8_t  color_map_type = 0;
		std::uint8_t  image_type = 2; // uncompressed true-color image.
		std::uint8_t  color_map_spec[5];
		std::uint16_t origin[2];
		std::uint16_t size[2];
		std::uint8_t  bits_per_pixel = 3*8;
		std::uint8_t  image_descriptor = 0;
	}
#ifdef __LINUX__
  __attribute__((packed))
#endif
    ;
#ifdef __WIN32__
	#pragma pack(pop)
#endif

Image::Image() : m_width(0), m_height(0)
{ }

Image::Image(int width, int height) : 
    m_width(width), 
    m_height(height), 
    m_pixels(width* height)
{ }

void Image::setSize(int width, int height)
{
    m_width = width;
    m_height = height;
    m_pixels.resize(m_width * m_height);
}

const glm::vec4& Image::getPixel(int i, int j) const
{
    cg_assert(i >= 0);
    cg_assert(j >= 0);
    cg_assert(i < m_width);
    cg_assert(j < m_height);
    return m_pixels[i + j * m_width];
}
	
const glm::vec4* Image::getPixels() const
{
    return m_pixels.data();
}

glm::vec4* Image::getPixels()
{
    return m_pixels.data();
}

void Image::setPixel(int i, int j, const glm::vec4& pixel)
{
    cg_assert(i >= 0);
    cg_assert(j >= 0);
    cg_assert(i < m_width);
    cg_assert(j < m_height);
    m_pixels[i + j * m_width] = pixel;
}

float const* Image::raw_data() const
{
    return reinterpret_cast<float const*>(m_pixels.data());
}

void Image::clear(glm::vec4 const& color)
{
    for (int i = 0; i < m_width * m_height; i++)
    {
        m_pixels[i] = color;
    }
}

void Image::saveTGA(std::string const& path, float gamma) const
{
    // Convert float RGBA to uint8 BGR.
    std::vector<std::uint8_t> bgr(m_pixels.size() * 3);
    for (std::size_t i = 0; i < m_pixels.size(); ++i)
    {
        std::size_t const idx = i * 3;
        for (int c = 0; c < 3; ++c)
        {
            float const gamma_corrected = std::pow(m_pixels[i][2-c], 1.f / gamma);
            float const mapped          = std::max<float>(0.0f, std::min<float>(255.0f, 255.f * gamma_corrected));
            bgr[idx+c]                  = uint8_t(mapped);
        }
    }

    TgaHeader header;
    header.size[0] = m_width;
    header.size[1] = m_height;

    std::ofstream f(path.c_str(), std::ios::binary);
    if (!f)
    {
        std::cerr << "Cannot write image to '" << path << "'." << std::endl;
        return;
    }

    f.write(reinterpret_cast<char const*>(&header), sizeof(header));
    f.write(reinterpret_cast<char const*>(bgr.data()), bgr.size());
}

void Image::load(std::string const& path, float gamma)
{
	int num_components;
	stbi_ldr_to_hdr_gamma(gamma);
    float *data = stbi_loadf(path.c_str(), &m_width, &m_height, &num_components, 4);
	if(!data) {
		std::cerr << "error: could not load image \"" << path << "\"" << std::endl;
		return;
	}
	m_pixels.resize(m_width * m_height);
	/* flip image in Y */
	for(int y = 0; y < m_height; y++) {
		memcpy(&m_pixels[(m_height - y - 1) * m_width],
				data + y * m_width * 4,
				4 * m_width * sizeof(float));
	}
	stbi_image_free(data);
}

