#pragma once

#include <glm/glm.hpp>

template <class T>
T interpolate_barycentric(T const& a, T const& b, T const& c, glm::vec3 const& bary)
{
    return a*bary.x + b*bary.y + c*bary.z;
}


/*
 *   v01 -------- weights.x --- v11
 *    |                          |
 * weights.y                  weights.y
 *    |                          |
 *   v00 -------- weights.x --- v10
 * */
template <class T>
T interpolate_bilinear(T const& v00, T const& v10, T const& v11, T const& v01, glm::vec2 const& weights)
{
    return glm::mix(
            glm::mix(v00, v10, weights.x),
            glm::mix(v01, v11, weights.x), weights.y);
}
