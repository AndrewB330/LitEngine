#include <lit/rendering/opengl/uniform_buffer.hpp>
#include <GL/glew.h>
#include <spdlog/spdlog.h>
#include <lit/rendering/opengl/utils.hpp>

using namespace lit::rendering::opengl;

UniformBuffer UniformBuffer::Create(UniformBufferInfo buffer_info) {
    return UniformBuffer(buffer_info);
}

UniformBuffer::~UniformBuffer() {
    if (m_buffer_id) {
        GL_CALL(glUnmapNamedBuffer(*m_buffer_id));
        GL_CALL(glDeleteBuffers(1, m_buffer_id.get()));
        m_buffer_id.reset();
    }
}

UniformBuffer::UniformBuffer(UniformBufferInfo buffer_info):m_size(buffer_info.size) {
    m_buffer_id = std::make_unique<uint32_t>();
    GL_CALL(glGenBuffers(1, m_buffer_id.get()));
    GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, *m_buffer_id));
    uint32_t mask = GL_MAP_PERSISTENT_BIT;
    if (buffer_info.access == UniformBufferAccess::Write) {
        mask |= GL_MAP_WRITE_BIT;
    } else if (buffer_info.access == UniformBufferAccess::Read) {
        mask |= GL_MAP_READ_BIT;
    } else if (buffer_info.access == UniformBufferAccess::ReadAndWrite) {
        mask |= GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
    }
    if (buffer_info.coherent) {
        mask |= GL_MAP_COHERENT_BIT;
    }
    GL_CALL(glBufferStorage(GL_SHADER_STORAGE_BUFFER, buffer_info.size, buffer_info.data,
                            mask | GL_DYNAMIC_STORAGE_BIT));

    if (buffer_info.flushable) {
        mask |= GL_MAP_FLUSH_EXPLICIT_BIT;
    }

    GL_CALL(m_host_ptr = glMapNamedBufferRange(*m_buffer_id, 0, m_size, mask | GL_MAP_UNSYNCHRONIZED_BIT));
}

void *UniformBuffer::GetHostPtr() const {
    return m_host_ptr;
}

void UniformBuffer::Bind(int index) const {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, *m_buffer_id);
}
