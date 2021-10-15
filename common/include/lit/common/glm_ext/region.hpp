#pragma once

#define GLM_FORCE_SWIZZLE

#include <glm/vec3.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/extended_min_max.hpp>

namespace lit::common::glm_ext {

    struct iregion3 {
        glm::ivec3 begin{};
        glm::ivec3 end{};

        iregion3(glm::ivec3 begin, glm::ivec3 end) : begin(begin), end(end) {}

        int size() const {
            if (glm::any(glm::lessThanEqual(end, begin)))
                return 0;
            if (glm::compMul(glm::dvec3(end - begin)) > std::numeric_limits<int>::max())
                return std::numeric_limits<int>::max();
            return glm::compMul(end - begin);
        }

        iregion3 clamped(iregion3 other) const {
            return iregion3(glm::clamp(begin, other.begin, other.end - 1), glm::clamp(end, other.begin + 1, other.end));
        }

        // todo: not quite correct :/
        static iregion3 empty() {
            return iregion3(glm::ivec3(4096),
                            glm::ivec3(-4096));
        }

        // todo: not quite correct :/
        static iregion3 all() {
            return iregion3(glm::ivec3(-4096),
                           glm::ivec3(4096));
        }

        void populate(glm::ivec3 pos) {
            begin = glm::min(begin, pos);
            end = glm::max(end, pos + 1);
        }

        iregion3 scaled_down() const {
            return iregion3(begin >> 1, (end + 1) >> 1);
        }
    };

}