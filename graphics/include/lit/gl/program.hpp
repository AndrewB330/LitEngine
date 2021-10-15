#pragma once

#include <vector>
#include <optional>
#include "shader.hpp"
#include "vertex_buffer.hpp"

namespace lit::gl {

    enum class FaceCulling : GLenum {
        Front = GL_FRONT,
        Back = GL_BACK,
        All = GL_FRONT_AND_BACK
    };

    enum class DepthFunc : GLenum {
        Never = GL_NEVER,
        Always = GL_ALWAYS,
        Less = GL_LESS,
        Greater = GL_GREATER
    };

    enum class BlendFuncFactor : GLenum {
        One = GL_ONE,
        SrcColor = GL_SRC_COLOR,
        DstColor = GL_DST_COLOR,
        OneMinusSrcColor = GL_ONE_MINUS_SRC_COLOR,
        OneMinusDstColor = GL_ONE_MINUS_DST_COLOR,
        SrcAlpha = GL_SRC_ALPHA,
        DstAlpha = GL_DST_ALPHA,
        OneMinusSrcAlpha = GL_ONE_MINUS_SRC_ALPHA,
        OneMinusDstAlpha = GL_ONE_MINUS_DST_ALPHA
    };

    struct BlendFunc {
        BlendFuncFactor src_factor = BlendFuncFactor::One;
        BlendFuncFactor dst_factor = BlendFuncFactor::One;
    };

    struct ProgramInfo {
        std::optional<FaceCulling> culling = std::nullopt;
        std::optional<DepthFunc> depth_func = std::nullopt;
        std::optional<BlendFunc> blend_func = std::nullopt;
    };

    class Program final : public ContextObject {
    public:
        explicit Program(std::shared_ptr<Context> ctx, const ProgramInfo &info);

        ~Program();

        void AttachShader(const std::shared_ptr<Shader> &shader);

        // You can use this after attaching all shaders to free them.
        void Link();

        void Draw(const std::shared_ptr<VertexArray> &array, DrawMode mode);

        int GetUniformLocation(const std::string &name);

        template<typename T>
        void SetUniform(const std::string &name, T value) {
            SetUniform(GetUniformLocation(name), value);
        }

        template<typename T>
        void SetUniform(int location, T value);

        // Before executing any draw call you should specify the program you will be using
        void Use();

    protected:


        bool m_linked = false;
        GLuint m_program_id = 0;
        std::vector<std::shared_ptr<Shader>> m_shaders;

        ProgramInfo m_info;
    };

}
