#pragma once

#include <glm/glm.hpp>
#include <cstdint>
#include <cmath>
#include <vector>

/*
 * Functions that generate geometry.
 */
void generate_strip(
	std::uint32_t N,
	std::vector<glm::vec3>* vertices);

void generate_grid(
	std::uint32_t N,
	std::vector<glm::vec3>* vertices,
	std::vector<glm::uvec3>* indices);

/*
 * Functions that render geometry.
 */
void draw_triangles(
	std::vector<glm::vec3> const& vertices,
	std::vector<glm::vec3> const& colors);

void draw_triangle_strip(
	std::vector<glm::vec3> const& vertices, 
	std::vector<glm::vec3> const& colors);

void draw_indexed_triangles(
	std::vector<glm::vec3>  const& vertices,
	std::vector<glm::vec3>  const& colors,
	std::vector<glm::uvec3> const& indices);

/*
 * Functions that do color matching.
 */
float integrate_trapezoidal(
	std::vector<float> const& x, 
	std::vector<float> const& y);

glm::vec3 spectrum_to_rgb(
	std::vector<float> const& spectrum);

