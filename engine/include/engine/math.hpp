#pragma once
#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>
#include <lit/common/glm_ext/transform.hpp>

using Mat3 = glm::dmat3;
using Vec3 = glm::dvec3;
using Quat = glm::dquat;
using Transform = lit::common::glm_ext::dtransform3;

inline Vec3 RandomUnit() {
    double x = rand() * 2.0 / RAND_MAX - 1.0;
    double y = rand() * 2.0 / RAND_MAX - 1.0;
    double z = rand() * 2.0 / RAND_MAX - 1.0;
    if (x * x + y * y + z * z < 1e-3) {
        return RandomUnit();
    }
    return glm::normalize(Vec3(x, y, z));
}
