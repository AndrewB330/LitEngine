#include <lit/rendering/opengl/vertex_array.hpp>
#include <GL/glew.h>

using namespace lit::rendering::opengl;

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
        glVertexAttribPointer(i, (int) m_attribs[i].size, m_attribs[i].type, GL_FALSE, (int) m_stride,
                              (void *) ((char *) (nullptr) + m_attribs[i].offset));
        glEnableVertexAttribArray(i);
    }
}

void VertexArray::DisableAttribs() {
    for (size_t i = 0; i < m_attribs.size(); i++) {
        glDisableVertexAttribArray(i);
    }
}

GLenum GetDrawModeEnum(DrawMode mode) {
    switch (mode) {
        case DrawMode::Points:
            return GL_POINTS;
        case DrawMode::LineStrip:
            return GL_LINE_STRIP;
        case DrawMode::LineLoop:
            return GL_LINE_LOOP;
        case DrawMode::Lines:
            return GL_LINES;
        case DrawMode::Triangles:
            return GL_TRIANGLES;
        case DrawMode::TriangleStrip:
            return GL_TRIANGLE_STRIP;
        case DrawMode::TriangleFan:
            return GL_TRIANGLE_FAN;
    }
    return GL_POINTS;
}

void VertexArray::Draw(DrawMode mode) {
    if (mode == DrawMode::Lines) {
        LIT_ENSURE(m_count % 2 == 0, spdlog::default_logger())
    }
    if (mode == DrawMode::Triangles || mode == DrawMode::TriangleFan || mode == DrawMode::TriangleStrip) {
        LIT_ENSURE(m_count % 3 == 0, spdlog::default_logger())
    }

    glBindVertexArray(m_vertex_array_id);
    if (m_is_elements) {
        glDrawElements(GetDrawModeEnum(mode), (int) m_count, GL_UNSIGNED_INT, nullptr);
    } else {
        glDrawArrays(GetDrawModeEnum(mode), 0, (int) m_count);
    }
}

void VertexArray::CreateGlVertexArray(void *vertex_data, uint32_t vertex_data_size) {
    glGenVertexArrays(1, &m_vertex_array_id);
    glGenBuffers(1, &m_vertex_buffer_id);

    glBindVertexArray(m_vertex_array_id);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, vertex_data_size, vertex_data, GL_STATIC_DRAW);

    EnableAttribs();
    glBindVertexArray(0);
    DisableAttribs();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void VertexArray::CreateGlVertexArrayWithIndices(void *vertex_data, uint32_t vertex_data_size, void *index_data,
                                                 uint32_t index_data_size) {
    glGenVertexArrays(1, &m_vertex_array_id);
    glGenBuffers(1, &m_vertex_buffer_id);
    glGenBuffers(1, &m_element_buffer_id);

    glBindVertexArray(m_vertex_array_id);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, vertex_data_size, vertex_data, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_element_buffer_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_data_size, index_data, GL_STATIC_DRAW);

    EnableAttribs();
    glBindVertexArray(0);
    DisableAttribs();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
