#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <memory>

class Image
{
public:
	Image();
	Image(int width, int height);

	int getWidth() const { return m_width; }
	int getHeight() const { return m_height; }

	void setSize(int width, int height);

	const glm::vec4& getPixel(int i, int j) const;
	const glm::vec4* getPixels() const;

	void setPixel(int i, int j, const glm::vec4& pixel);
	glm::vec4* getPixels();

	float const* raw_data() const;
	void clear(glm::vec4 const& color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

	void saveTGA(std::string const& path, float gamma) const;
	void load(std::string const& path, float gamma);

	enum WrapMode { ZERO, CLAMP, REPEAT };
	glm::vec4 getPixel(int i, int j, WrapMode wrap_mode) const;

	static void create_gaussian_kernel_1d(float sigma, int kernel_size, float* kernel);
	static void create_gaussian_kernel_2d(float sigma, int kernel_size, float* kernel);

	std::shared_ptr<Image> filter          (int kernel_size, float* kernel, WrapMode wrap_mode = CLAMP) const;
	std::shared_ptr<Image> filter_separable(int kernel_size, float* kernel, WrapMode wrap_mode = CLAMP) const;

	std::shared_ptr<Image> filter_gaussian(float sigma, int kernel_size, WrapMode wrap_mode = CLAMP) const;
	std::shared_ptr<Image> filter_gaussian_separable(float sigma, int kernel_size, WrapMode wrap_mode = CLAMP) const;

protected:
	int m_width;
	int m_height;
	std::vector<glm::vec4> m_pixels;
};

