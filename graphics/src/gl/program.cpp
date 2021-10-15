#include <lit/gl/program.hpp>

using namespace lit::gl;

Program::Program(std::shared_ptr<Context> ctx, const ProgramInfo &info) : ContextObject(std::move(ctx)), m_info(info) {
    m_program_id = glCreateProgram(); // todo: log error
}

Program::~Program() {
    if (m_program_id) {
        glDeleteProgram(m_program_id);
    }
}

int Program::GetUniformLocation(const std::string &name) {
    auto location = glGetUniformLocation(m_program_id, name.c_str());
    LIT_ASSERT_NOTHROW(location != -1, fmt::format("Variable with name \"{}\" wasn't found", name), m_ctx->GetLogger())
    return location;
}

void Program::Use() {
    Link();

    if (!m_program_id) {
        return;
    }

    GLint active_program_id = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &active_program_id);
    if (active_program_id == m_program_id) {
        return;
    }

    glUseProgram(m_program_id);

    if (m_info.culling) {
        glEnable(GL_CULL_FACE);
        glCullFace((GLenum)m_info.culling.value());
    } else {
        glDisable(GL_CULL_FACE);
    }

    if (m_info.depth_func) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc((GLenum)m_info.depth_func.value());
    } else {
        glDisable(GL_DEPTH_TEST);
    }

    if (m_info.blend_func) {
        glEnable(GL_BLEND);
        glBlendFunc((GLenum)m_info.blend_func->src_factor, (GLenum)m_info.blend_func->dst_factor);
    } else {
        glDisable(GL_BLEND);
    }
}

void Program::AttachShader(const std::shared_ptr<Shader> &shader) {
    glAttachShader(m_program_id, shader->m_shader_id);
    m_shaders.push_back(shader);
}

void Program::Link() {
    if (!m_linked && m_program_id) {
        glLinkProgram(m_program_id);
        m_linked = true;
    }
}

void Program::Draw(const std::shared_ptr<VertexArray> &array, DrawMode mode) {
    Use();
    array->Draw(mode);
}

template<>
void Program::SetUniform(int location, float value) {
    glProgramUniform1f(m_program_id, location, value);
}

template<>
void Program::SetUniform(int location, int value) {
    glProgramUniform1i(m_program_id, location, value);
}

template<>
void Program::SetUniform(int location, glm::ivec3 value) {
    glUniform3i(location, value.x, value.y, value.z);
}

template<>
void Program::SetUniform(int location, glm::ivec2 value) {
    glUniform2i(location, value.x, value.y);
}

template<>
void Program::SetUniform(int location, glm::vec3 value) {
    glUniform3f(location, value.x, value.y, value.z);
}

template<>
void Program::SetUniform(int location, glm::vec2 value) {
    glUniform2f(location, value.x, value.y);
}

template<>
void Program::SetUniform(int location, glm::mat4 value) {
    glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
}
