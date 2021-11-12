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

        uint64_t volume() const {
            if (glm::any(glm::lessThanEqual(end, begin)))
                return 0;
            if (glm::compMul(glm::dvec3(end - begin)) > (double)std::numeric_limits<uint64_t>::max())
                return std::numeric_limits<uint64_t>::max();
            return glm::compMul( glm::vec<3, int64_t>(end - begin));
        }

        static iregion3 empty() {
            return {glm::ivec3(std::numeric_limits<int>::max()), glm::ivec3(std::numeric_limits<int>::min())};
        }

        static iregion3 all() {
            return {glm::ivec3(std::numeric_limits<int>::min()), glm::ivec3(std::numeric_limits<int>::max())};
        }

        iregion3 clamped(iregion3 other) const {
            return {glm::clamp(begin, other.begin, other.end - 1), glm::clamp(end, other.begin + 1, other.end)};
        }

        iregion3 scaled_down() const {
            return {begin >> 1, (end + 1) >> 1};
        }

        void populate(glm::ivec3 pos) {
            begin = glm::min(begin, pos);
            end = glm::max(end, pos + 1);
        }
    };

}