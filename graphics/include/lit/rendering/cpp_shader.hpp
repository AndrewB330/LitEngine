#pragma once

#define GLM_FORCE_SWIZZLE

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

#include <lit/gl/shader.hpp>
#include <lit/gl/frame_buffer.hpp>
#include <lit/rendering/cpp_shader_utils.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/extended_min_max.hpp>
#include <lit/gl/cubemap.hpp>

#define uniform(type, name) Uniform<type> name = Uniform<type>(#name, *this);
#define uniform_field(type, name) Uniform<type> name = Uniform<type>(#name, *this);
#define uniform_struct(type, name) UniformStruct<type> name = UniformStruct<type>(#name, *this);

#define shader_out_regular(location, type, name, ...) \
    int name ## _h; private: type name; public:

#define shader_out_texture(location, type, name, fb_attachment, ...) \
    std::weak_ptr<lit::gl::Texture2D> name ## _texture; \
    private: type name; void name ## _helper() {m_output_textures.emplace_back(&name ## _texture); m_outputs.push_back(fb_attachment);} int name ## _i = (name ## _helper(), 0); public:

#define GET_SHADER_OUT_MACRO(_1,_2,_3,_4,NAME,...) NAME
#define shader_out(...) GET_SHADER_OUT_MACRO(__VA_ARGS__, shader_out_texture, shader_out_regular)(__VA_ARGS__)

#define shader_in(location, type, name) const type name = type();

#define shader_vert(name) \
class name ## _base : public lit::rendering::VertShader { \
    std::string GetName() const { return #name; }             \
    std::string GetLocation() const { return __FILE__; }\
};\
struct name : public name ## _base

#define shader_frag(name) \
class name ## _base : public lit::rendering::FragShader { \
    std::string GetName() const { return #name; }             \
    std::string GetLocation() const { return __FILE__; }\
};\
struct name : public name ## _base

#define REGISTER_OP(op) \
    template<typename T, typename S>\
    friend decltype(T() op S()) operator op(const Uniform<T> & lhs, const Uniform<S> & rhs) {\
        return lhs.m_value op rhs.m_value;\
    }                   \
    template<typename T, typename S>\
    friend decltype(T() op S()) operator op(const Uniform<T> & lhs, const S & rhs) {\
        return lhs.m_value op rhs;\
    }                   \
    template<typename T, typename S>\
    friend decltype(T() op S()) operator op(const T & lhs, const Uniform<S> & rhs) {\
        return lhs op rhs.m_value;\
    }

#define REGISTER_FUNC(func) \
    template<typename T, typename S>\
    friend decltype(func(T(), S())) func(const Uniform<T> & lhs, const Uniform<S> & rhs) {\
        return func(lhs.m_value, rhs.m_value);\
    } \
    template<typename T, typename S>\
    friend decltype(func(T(), S())) func(const Uniform<T> & lhs, const S & rhs) {\
        return func(lhs.m_value, rhs);\
    } \
    template<typename T, typename S>\
    friend decltype(func(T(), S())) func(const T & lhs, const Uniform<S> & rhs) {\
        return func(lhs, rhs.m_value);\
    } \

namespace lit::rendering {

    class CppShader : public UniformHolder {
    public:
        CppShader() = default;

        virtual ~CppShader() = default;

        std::string GetGlslCode();

        template<typename T, typename V>
        friend class PipelineNode;

    protected:

        virtual std::string GetLocation() const;

        virtual std::string GetName() const;

        // This structure is used as a wrapper for standard and glm:: types
        // With this structure we can write shader.matrix = mat4(), and it will
        // automatically send the new uniform value to shader
        // But also we can use it in regular cpp-shader code (e.g. dot(vec3, Uniform<vec3>))
        // Because of standard glm:: templates can't recognize uniform type, we should decalre all operators
        // and function that we want to use with Uniforms using macros REGISTER_OP, REGISTER_FUNC etc.
        template<typename T>
        struct Uniform {
            T m_value = T(); // current value of uniform variable
            GLint m_location = -1; // location of uniform variable

            operator T() { return m_value; } // NOLINT(google-explicit-constructor)

            Uniform(const std::string &name, UniformHolder &owner) {
                owner.m_uniform_locations.emplace_back(name, &m_location);
            }

            Uniform() = default;

            Uniform<T> &operator=(const T &value) {
                if (value != m_value) {
                    m_value = value;
                    SetUniform(m_location, m_value);
                }
                return *this;
            }

            template<typename T_ = T, typename = std::enable_if_t<std::is_base_of<base_sampler2D, T_>::value>>
            Uniform<T> &operator=(const std::weak_ptr<lit::gl::Texture2D> &texture) {
                // todo: check internal type in runtime
                texture.lock()->Bind(m_location + 1);
                SetUniform(m_location, m_location + 1);
                return *this;
            }

            template<typename T_ = T, typename = std::enable_if_t<std::is_base_of<base_sampler3D, T_>::value>>
            Uniform<T> &operator=(const std::weak_ptr<lit::gl::Texture3D> &texture) {
                // todo: check internal type in runtime
                texture.lock()->Bind(m_location + 1);
                SetUniform(m_location, m_location + 1);
                return *this;
            }

            template<typename T_ = T, typename = std::enable_if_t<std::is_base_of<samplerCube, T_>::value>>
            Uniform<T> &operator=(const std::weak_ptr<lit::gl::Cubemap> &texture) {
                // todo: check internal type in runtime
                texture.lock()->Bind(m_location + 1);
                SetUniform(m_location, m_location + 1);
                return *this;
            }



            template<typename T_ = T, typename = std::enable_if_t<std::is_base_of<base_sampler2D, T_>::value>>
            void Bind(const std::weak_ptr<lit::gl::Texture2D> &texture, int location) {
                // todo: REMOVE
                texture.lock()->Bind(location);
                SetUniform(m_location, location);
            }

            template<typename T_ = T, typename = std::enable_if_t<std::is_base_of<base_sampler3D, T_>::value>>
            void Bind(const std::weak_ptr<lit::gl::Texture3D> &texture, int location) {
                // todo: REMOVE
                texture.lock()->Bind(location);
                SetUniform(m_location, location);
            }

            template<typename T_ = T, typename = std::enable_if_t<std::is_base_of<samplerCube, T_>::value>>
            void Bind(const std::weak_ptr<lit::gl::Cubemap> &texture, int location) {
                // todo: REMOVE
                texture.lock()->Bind(location);
                SetUniform(m_location, location);
            }
        };

        // Wrapper structure, the same role as Uniform<T>, but for custom structures
        // fields of which can be accessed by shader.custom_struct.field = value;
        // It will automatically update uniform value of "custom_struct.field" variable
        template<typename T>
        struct UniformStruct : public T {
            UniformStruct(const std::string &name, CppShader &owner) {
                owner.m_uniform_struct_holders.emplace_back(name, this);
            }

            UniformStruct() = default;
        };

        REGISTER_OP(*);

        REGISTER_OP(+);

        REGISTER_OP(/);

        REGISTER_OP(%);

        REGISTER_OP(-);

        REGISTER_FUNC(lessThan);

        std::vector<std::pair<std::string, UniformHolder*>> m_uniform_struct_holders;
    };

    class VertShader : public CppShader {
    public:
        VertShader() = default;

        ~VertShader() override = default;

    protected:
        glm::vec4 gl_Position{}; // default out variable
    };

    class FragShader : public CppShader {
    public:
        FragShader() = default;

        ~FragShader() override = default;

        std::vector<gl::Attachment> GetOutputTypes() const;

        template<typename T, typename V>
        friend class PipelineNode;

    protected:
        std::vector<gl::Attachment> m_outputs;
        std::vector<std::weak_ptr<gl::Texture2D>*> m_output_textures;

        glm::vec4 gl_FragCoord{}; // default in variable

        void discard();
    };

}
