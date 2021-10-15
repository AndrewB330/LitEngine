#pragma once

namespace lit::rendering {

    template<typename T>
    void SetUniform(GLint location, T val);

    struct UniformHolder {
        std::vector<std::pair<std::string, GLint*>> m_uniform_locations;
    };

    struct base_sampler {
    };

    struct base_sampler2D : public base_sampler {
    };

    struct base_sampler3D : public base_sampler {
    };

    struct sampler2D : public base_sampler2D {
    };

    struct usampler2D : public base_sampler2D {
    };

    struct sampler3D : public base_sampler3D {
    };

    struct usampler3D : public base_sampler3D {
    };

    struct samplerCube : public base_sampler {

    };

    glm::uvec4 texelFetch(usampler3D s, glm::ivec3 uv, int level);

    glm::vec4 texelFetch(sampler3D s, glm::ivec3 uv, int level);

    glm::vec4 texelFetch(sampler2D s, glm::ivec2 uv, int level);

    glm::uvec4 texture(usampler2D s, glm::vec2 uv, int level);

    glm::vec4 texture(sampler2D s, glm::vec2 uv, int level);

    glm::vec4 texture(samplerCube s, glm::vec3 uv);

}

namespace glm {
    // Some additional operator overloads for glm types that are available in GLSL language,
    // but are not defined in glm library.

    template<typename T, typename S, int DIM, typename R = decltype(T() -
                                                                    S()), typename = std::enable_if_t<!std::is_same<T, S>::value>>
    vec <DIM, R> operator-(const vec <DIM, T> &lhs, const vec <DIM, S> &rhs) {
        return vec<DIM, R>(lhs) - vec<DIM, R>(rhs);
    }

    template<typename T, typename S, int DIM, typename R = decltype(T() +
                                                                    S()), typename = std::enable_if_t<!std::is_same<T, S>::value>>
    vec <DIM, R> operator+(const vec <DIM, T> &lhs, const vec <DIM, S> &rhs) {
        return vec<DIM, R>(lhs) + vec<DIM, R>(rhs);
    }

    template<typename T, typename S, int DIM, typename R = decltype(T() *
                                                                    S()), typename = std::enable_if_t<!std::is_same<T, S>::value>>
    vec <DIM, R> operator*(const vec <DIM, T> &lhs, const vec <DIM, S> &rhs) {
        return vec<DIM, R>(lhs) * vec<DIM, R>(rhs);
    }

    template<typename T, typename S, int DIM, typename R = decltype(T() *
                                                                    S()), typename = std::enable_if_t<!std::is_same<T, S>::value>, typename = std::enable_if_t<std::is_arithmetic<S>::value>>
    vec <DIM, R> operator*(const vec <DIM, T> &lhs, const S &rhs) {
        return vec<DIM, R>(lhs) * vec<DIM, R>(rhs);
    }

}