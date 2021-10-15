#include <lit/gl/vertex_buffer.hpp>

using namespace lit::gl;

template<>
GLenum VertexArray::GetGlType<float>() {
    return GL_FLOAT;
}

template<>
GLenum VertexArray::GetGlType<int>() {
    return GL_INT;
}

void VertexArray::EnableAttribs() {
    for (size_t i = 0; i < m_attribs.size(); i++) {
        glVertexAttribPointer(i, m_attribs[i].size, m_attribs[i].type, GL_FALSE, (int) m_stride,
                              (void *) ((char *) (nullptr) + m_attribs[i].offset));
        glEnableVertexAttribArray(i);
    }
}

void VertexArray::DisableAttribs() {
    for (size_t i = 0; i < m_attribs.size(); i++) {
        glDisableVertexAttribArray(i);
    }
}

void VertexArray::Draw(DrawMode mode) {
    if (mode == DrawMode::Lines) {
        LIT_ENSURE(m_count % 2 == 0, m_ctx->GetLogger())
    }
    if (mode == DrawMode::Triangles || mode == DrawMode::TriangleFan || mode == DrawMode::TriangleStrip) {
        LIT_ENSURE(m_count % 3 == 0, m_ctx->GetLogger())
    }

    glBindVertexArray(m_vertex_array_id);
    if (m_is_elements) {
        glDrawElements((GLenum) mode, (int) m_count, GL_UNSIGNED_INT, nullptr);
    } else {
        glDrawArrays((GLenum) mode, 0, (int) m_count);
    }
}
