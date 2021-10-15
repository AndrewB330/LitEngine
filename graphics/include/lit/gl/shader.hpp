#pragma once

#include <string>
#include <memory>
#include "context.hpp"

namespace lit::gl {

    enum class ShaderType : GLenum {
        Undefined = 0,
        Vertex = GL_VERTEX_SHADER,
        Fragment = GL_FRAGMENT_SHADER,
        Geometry = GL_GEOMETRY_SHADER,
        Compute = GL_COMPUTE_SHADER
    };

    // Shader compiled from source code
    class Shader final : public ContextObject {
    public:
        Shader(std::shared_ptr<Context> ctx, ShaderType type, const std::string &source_code);

        ~Shader();

    protected:

        friend class Program;

        friend class Context;

        GLuint m_shader_id;
        ShaderType m_type;
    };

}