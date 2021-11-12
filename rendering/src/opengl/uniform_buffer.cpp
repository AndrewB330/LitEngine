#include <lit/rendering/opengl/uniform_buffer.hpp>
#include <GL/glew.h>
#include <spdlog/spdlog.h>
#include <lit/rendering/opengl/utils.hpp>

using namespace lit::rendering::opengl;

UniformBuffer UniformBuffer::Create(uint64_t size, void *data) {
    return UniformBuffer(size, data);
}

UniformBuffer::~UniformBuffer() {
    if (m_buffer_id) {
        GL_CALL(glUnmapNamedBuffer(*m_buffer_id));
        GL_CALL(glDeleteBuffers(1, m_buffer_id.get()));
        m_buffer_id.reset();
    }
}

UniformBuffer::UniformBuffer(uint64_t size, void *data):m_size(size) {
    m_buffer_id = std::make_unique<uint32_t>();
    GL_CALL(glGenBuffers(1, m_buffer_id.get()));
    GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, *m_buffer_id));
    GL_CALL(glBufferStorage(GL_SHADER_STORAGE_BUFFER, size, data,
                            GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_DYNAMIC_STORAGE_BIT));
    GL_CALL(m_host_ptr = glMapNamedBufferRange(*m_buffer_id, 0, m_size,
                                       GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_PERSISTENT_BIT));
}

void *UniformBuffer::GetHostPtr() {
    return m_host_ptr;
}

void UniformBuffer::Bind(int index) {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, *m_buffer_id);
}
