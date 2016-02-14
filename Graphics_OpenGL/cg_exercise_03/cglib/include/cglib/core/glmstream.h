#pragma once

#include <glm/glm.hpp>

#include <iostream>
#include <ostream>

inline std::ostream& operator<<(std::ostream& os, glm::vec4 const& vec)
{
	os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
	return os;
}

inline std::ostream& operator<<(std::ostream& os, glm::vec3 const& vec)
{
	os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
	return os;
}

inline std::ostream& operator<<(std::ostream& os, glm::vec2 const& vec)
{
	os << "(" << vec.x << ", " << vec.y << ")";
	return os;
}

inline std::ostream& operator<<(std::ostream& os, glm::mat4 const& mat)
{
	os << mat[0][0] << ", " << mat[0][1] << ", " << mat[0][2] << ", " << mat[0][3] << "\n";
	os << mat[1][0] << ", " << mat[1][1] << ", " << mat[1][2] << ", " << mat[1][3] << "\n";
	os << mat[2][0] << ", " << mat[2][1] << ", " << mat[2][2] << ", " << mat[2][3] << "\n";
	os << mat[3][0] << ", " << mat[3][1] << ", " << mat[3][2] << ", " << mat[3][3] << "\n";
	return os;
}
