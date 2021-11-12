#include <lit/rendering/opengl/shader.hpp>
#include <spdlog/spdlog.h>
#include <GL/glew.h>
#include <fstream>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <lit/rendering/opengl/utils.hpp>

using namespace lit::rendering::opengl;

ComputeShader::ComputeShader(uint32_t program_id) : m_program_id(std::make_unique<uint32_t>(program_id)) {
    glGetProgramiv(program_id, GL_COMPUTE_WORK_GROUP_SIZE, &m_local_group_size.x);
}

std::optional<ComputeShader> ComputeShader::TryCreate(const std::string &shader_path) {
    auto program_id = GL_CALL(glCreateProgram());

    std::ifstream fin(shader_path);
    std::string shader_sources = {(std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>()};
    auto shader_sources_c_str = shader_sources.c_str();

    GL_CALL(auto shader_id = glCreateShader(GL_COMPUTE_SHADER));

    GL_CALL(glShaderSource(shader_id, 1, &shader_sources_c_str, nullptr));
    GL_CALL(glCompileShader(shader_id));

    int success;
    GL_CALL(glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success));
    if (!success) {
        char message[512];
        GL_CALL(glGetShaderInfoLog(shader_id, sizeof(message), nullptr, message));
        spdlog::default_logger()->error(message);
        return std::nullopt;
    } else {
        spdlog::default_logger()->trace("Compute shader compiled successfully");
    }

    GL_CALL(glAttachShader(program_id, shader_id));

    GL_CALL(glLinkProgram(program_id));

    GLint isLinked = 0;
    GL_CALL(glGetProgramiv(program_id, GL_LINK_STATUS, &isLinked));
    if (isLinked == GL_FALSE) {
        GLint maxLength = 0;
        GL_CALL(glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &maxLength));
        std::string message(maxLength, ' ');
        GL_CALL(glGetProgramInfoLog(program_id, maxLength, &maxLength, &message[0]));
        spdlog::default_logger()->error(message);
        GL_CALL(glDeleteProgram(program_id));
        return std::nullopt;
    }

    GL_CALL(glDeleteShader(shader_id));

    return ComputeShader(program_id);
}

ComputeShader::~ComputeShader() {
    if (m_program_id) {
        GL_CALL(glDeleteProgram(*m_program_id));
        m_program_id.reset();
        spdlog::default_logger()->trace("Shader program destroyed");
    }
}

void ComputeShader::Bind() {
    GL_CALL(glUseProgram(*m_program_id));
}

void ComputeShader::Unbind() {
    GL_CALL(glUseProgram(0));
}

int ComputeShader::GetUniformLocation(const std::string &name) {
    if (m_location_cache.find(name) != m_location_cache.end()) {
        return m_location_cache[name];
    }

    GL_CALL(auto location = glGetUniformLocation(*m_program_id, name.c_str()));
    if (location == -1) {
        spdlog::default_logger()->warn("Variable with name \"{}\" wasn't found", name);
    }
    return m_location_cache[name] = location;
}

ComputeShader ComputeShader::Create(const std::string &shader_path) {
    auto res = TryCreate(shader_path);
    if (!res) {
        // todo: invalid shader?
        return ComputeShader(0);
    }
    return std::move(*res);
}

glm::ivec3 ComputeShader::GetLocalGroupSize() const {
    return m_local_group_size;
}

void ComputeShader::Dispatch(glm::ivec3 total_size) {
    auto groups = (total_size + m_local_group_size - glm::ivec3(1)) / m_local_group_size;
    glDispatchCompute((GLuint) groups.x, (GLuint) groups.y, (GLuint) groups.z);
}

template<>
void ComputeShader::SetUniform(int location, float value) {
    GL_CALL(glProgramUniform1f(*m_program_id, location, value));
}

template<>
void ComputeShader::SetUniform(int location, int value) {
    GL_CALL(glProgramUniform1i(*m_program_id, location, value));
}

template<>
void ComputeShader::SetUniform(int location, glm::ivec3 value) {
    GL_CALL(glUniform3i(location, value.x, value.y, value.z));
}

template<>
void ComputeShader::SetUniform(int location, glm::ivec2 value) {
    GL_CALL(glUniform2i(location, value.x, value.y));
}

template<>
void ComputeShader::SetUniform(int location, glm::vec3 value) {
    GL_CALL(glUniform3f(location, value.x, value.y, value.z));
}

template<>
void ComputeShader::SetUniform(int location, glm::vec2 value) {
    GL_CALL(glUniform2f(location, value.x, value.y));
}

template<>
void ComputeShader::SetUniform(int location, glm::mat4 value) {
    GL_CALL(glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]));
}
