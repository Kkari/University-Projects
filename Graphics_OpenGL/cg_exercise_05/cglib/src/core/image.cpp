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

glm::vec4 Image::getPixel(int i, int j, WrapMode wrap_mode) const
{
	switch (wrap_mode) {
		case ZERO:
			if (i < 0 || i >= m_width || j < 0 || j >= m_height)
				return glm::vec4(0.f);
		break;
		case CLAMP:
			i = std::min(m_width-1,  std::max(0, i));
			j = std::min(m_height-1, std::max(0, j));
		break;
		case REPEAT:
			while (i < 0)
				i += m_width;
			i = i % m_width;
			while (j < 0)
				j += m_height;
			j = j % m_height;
		break;
	}
	cg_assert(i >= 0);
	cg_assert(j >= 0);
	cg_assert(i < m_width);
	cg_assert(j < m_height);
	return m_pixels[i + j * m_width];
}

std::shared_ptr<Image> Image::filter_gaussian(float sigma, int kernel_size, WrapMode wrap_mode) const
{
	std::vector<float> kernel(kernel_size*kernel_size);
	create_gaussian_kernel_2d(sigma, kernel_size, kernel.data());
	return filter(kernel_size, kernel.data(), wrap_mode);
}

std::shared_ptr<Image> Image::filter_gaussian_separable(float sigma, int kernel_size, WrapMode wrap_mode) const
{
	std::vector<float> kernel(kernel_size);
	create_gaussian_kernel_1d(sigma, kernel_size, kernel.data());
	return filter_separable(kernel_size, kernel.data(), wrap_mode);
}

/*
 * Create a 1 dimensional normalized gauss kernel
 *
 * Parameters:
 *  - sigma:       the parameter of the gaussian
 *  - kernel_size: the size of the kernel (has to be odd)
 *  - kernel:      an array with size kernel_size elements that
 *                 has to be filled with the kernel values
 *
 */
void Image::create_gaussian_kernel_1d(
		float sigma,
		int kernel_size,
		float* kernel)
{
	cg_assert(kernel_size%2==1);

	// TODO: calculate filter values as described on the exercise sheet. 
	// Make sure your kernel is normalized
	const float sigma_squared = sigma*sigma;
	float normalize = 0.f;
	for (int i = -kernel_size/2; i <= kernel_size/2; ++i) {
		const float value = std::exp(-(i*i)/(2*sigma_squared));
		normalize += value;
		kernel[i+kernel_size/2] = value;
	}

	for (int i = 0; i < kernel_size; ++i) {
		kernel[i] /= normalize;
	}
}

/*
 * Create a 2 dimensional quadratic and normalized gauss kernel
 *
 * Parameters:
 *  - sigma:       the parameter of the gaussian
 *  - kernel_size: the size of the kernel (has to be odd)
 *  - kernel:      an array with kernel_size*kernel_size elements that
 *                 has to be filled with the kernel values
 */
void Image::create_gaussian_kernel_2d(
		float sigma,
		int kernel_size,
		float* kernel)
{
	cg_assert(kernel_size%2==1);

	// TODO: calculate filter values as described on the exercise sheet. 
	// Make sure your kernel is normalized
	const float sigma_squared = sigma*sigma;
	float normalize = 0.f;
	for (int j = -kernel_size/2; j <= kernel_size/2; ++j) {
		for (int i = -kernel_size/2; i <= kernel_size/2; ++i) {
			const float value = std::exp(-(i*i+j*j)/(2*sigma_squared));
			normalize += value;
			kernel[(i+kernel_size/2)+(j+kernel_size/2)*kernel_size] = value;
		}
	}

	for (int j = 0; j < kernel_size; ++j) {
		for (int i = 0; i < kernel_size; ++i) {
			kernel[i+j*kernel_size] /= normalize;
		}
	}
}
	
/*
 * Convolve an image with a 2d filter kernel
 *
 * Parameters:
 *  - kernel_size: the size of the 2d-kernel
 *  - kernel:      the 2d-kernel with kernel_size*kernel_size elements
 *  - wrap_mode:   needs to be known to handle repeating 
 *                 textures correctly
 */
std::shared_ptr<Image> Image::
filter(int kernel_size, float* kernel, WrapMode wrap_mode) const
{
	cg_assert (kernel_size%2==1 && "kernel size should be odd.");
	cg_assert (kernel_size > 0 && "kernel size should be greater than 0.");

	std::shared_ptr<Image> dst_image = std::make_shared<Image>(m_width, m_height);

	// TODO: perform the naive 2d convolution here. 
	// use the methods getPixel(x, y, wrap_mode) and
	// setPixel(x, y, value) to get and set pixels of an image
	for (int y = 0; y < m_height; ++y)
	{
		for (int x = 0; x < m_width; ++x)
		{
			glm::vec4 val(0.f);
			for (int j = -kernel_size/2; j <= kernel_size/2; ++j) {
				for (int i = -kernel_size/2; i <= kernel_size/2; ++i) {
					const float weight = kernel[(i+kernel_size/2)+(j+kernel_size/2)*kernel_size];
					val += weight * getPixel(x+i, y+j, wrap_mode);
				}
			}
			dst_image->setPixel(x, y, val);
		}
	}

	return dst_image;
}

/*
 * Convolve an image with a separable 1d filter kernel
 *
 * Parameters:
 *  - kernel_size: the size of the 1d kernel
 *  - kernel:      the 1d-kernel with kernel_size elements
 *  - wrap_mode:   needs to be known to handle repeating 
 *                 textures correctly
 */
std::shared_ptr<Image> Image::
filter_separable(int kernel_size, float* kernel, WrapMode wrap_mode) const
{
	cg_assert (kernel_size%2==1 && "kernel size should be odd.");
	cg_assert (kernel_size > 0 && "kernel size should be greater than 0.");

	std::shared_ptr<Image> dst_image = std::make_shared<Image>(m_width, m_height);

	// TODO: realize the 2d convolution with two
	// convolutions of the image with a 1d-kernel.
	// convolve the image horizontally and then convolve
	// the result vertically (or vise-versa).
	//
	// use the methods getPixel(x, y, wrap_mode) and
	// setPixel(x, y, value) to get and set pixels of an image
	Image tmp_image(m_width, m_height);

	// filter in y direction
	for (int y = 0; y < m_height; ++y)
	{
		for (int x = 0; x < m_width; ++x)
		{
			glm::vec4 val(0.f);
			for (int dy = -kernel_size/2; dy <= kernel_size/2; ++dy) {
				const float weight = kernel[dy+kernel_size/2];
				val += weight * getPixel(x, y+dy, wrap_mode);
			}
			tmp_image.setPixel(x, y, val);
		}
	}
	// filter in x direction
	for (int y = 0; y < m_height; ++y)
	{
		for (int x = 0; x < m_width; ++x)
		{
			glm::vec4 val(0.f);
			for (int dx = -kernel_size/2; dx <= kernel_size/2; ++dx) {
				const float weight = kernel[dx+kernel_size/2];
				val += weight * tmp_image.getPixel(x+dx, y, wrap_mode);
			}
			dst_image->setPixel(x, y, val);
		}
	}
	return dst_image;
}

