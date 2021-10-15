#pragma once

#define GLM_FORCE_SWIZZLE

#include <glm/vec3.hpp>
#include <glm/gtx/extended_min_max.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

namespace lit::common::glm_ext {

    template<typename T>
    struct VecComp3 {
        bool operator()(const glm::vec<3, T> &lhs, const glm::vec<3, T> &rhs) const {
            if (lhs.x != rhs.x)
                return lhs.x < rhs.x;
            if (lhs.y != rhs.y)
                return lhs.y < rhs.y;
            return lhs.z < rhs.z;
        }
    };

}