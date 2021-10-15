#pragma once

#define GLM_FORCE_SWIZZLE

#include <glm/vec3.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

namespace lit::common::glm_ext {

    template<typename T, typename = std::enable_if_t<std::is_floating_point<T>::value>>
    struct ttransform3 {
        using tvec = glm::vec<3, T>;
        using tquat = glm::tquat<T>;
        using tmat4 = glm::mat<4, 4, T>;

        tvec translation = tvec();
        tquat rotation = glm::identity<tquat>(); // todo: check this
        T scale = 1.0;

        ttransform3() = default;

        ttransform3(tvec t, tquat r, double s) : translation(t), rotation(glm::normalize(r)), scale(s) {}

        tvec apply(const tvec &pos) const {
            return glm::rotate(rotation, pos * scale) + translation;
        }

        tvec apply_inv(const tvec &pos) const {
            auto rotation_inv = tquat(-rotation.w, rotation.x, rotation.y, rotation.z);
            return glm::rotate(rotation_inv, pos - translation) / scale;
        }

        tmat4 mat() const {
            return glm::translate(translation) *
                   glm::toMat4(rotation) *
                   glm::scale(tvec(scale));
        }

        tmat4 mat_inv() const {
            auto rotation_inv = tquat(-rotation.w, rotation.x, rotation.y, rotation.z);
            return glm::scale(tvec(1 / scale)) *
                   glm::toMat4(rotation_inv) *
                   glm::translate(-translation);
        }

        bool operator==(const ttransform3<T> & other) {
            return translation == other.translation && rotation == other.rotation && scale == other.scale;
        }
    };

    using transform3 = ttransform3<float>;
    using dtransform3 = ttransform3<double>;
    using ldtransform3 = ttransform3<long double>;
};