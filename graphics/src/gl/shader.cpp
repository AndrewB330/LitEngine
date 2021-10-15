#include <lit/gl/shader.hpp>
#include <iostream>

using namespace lit::gl;

Shader::Shader(std::shared_ptr<Context> ctx, ShaderType type, const std::string &source_code)
        : ContextObject(std::move(ctx)) {
    m_type = type;
    const char *source_c_str = source_code.c_str();
    m_shader_id = glCreateShader((GLenum) type);
    glShaderSource(m_shader_id, 1, &source_c_str, nullptr);
    glCompileShader(m_shader_id);

    int success;
    char infoLog[512];
    glGetShaderiv(m_shader_id, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(m_shader_id, 512, nullptr, infoLog);
        // todo: log
        std::cerr << infoLog << std::endl;
    }
}

Shader::~Shader() {
    if (m_shader_id) {
        glDeleteShader(m_shader_id);
    }
}
