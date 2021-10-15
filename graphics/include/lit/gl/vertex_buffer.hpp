#pragma once

#include <glm/glm.hpp>
#include <GL/glew.h>
#include "context.hpp"

namespace lit::gl {

    template<typename ...Attribs>
    struct VertexData {
        std::tuple<Attribs...> attribs;
    };

    enum class DrawMode : GLenum {
        Points = GL_POINTS,
        LineStrip = GL_LINE_STRIP,
        LineLoop = GL_LINE_LOOP,
        Lines = GL_LINES,
        Triangles = GL_TRIANGLES,
        TriangleStrip = GL_TRIANGLE_STRIP,
        TriangleFan = GL_TRIANGLE_FAN,
    };

    class VertexArray : public ContextObject {
    public:
        template<typename ...Attribs>
        VertexArray(const std::shared_ptr<Context> &ctx, const std::vector<VertexData<Attribs...>> &vertices,
                    const std::vector<uint32_t> &indices)
                : ContextObject(ctx), m_is_elements(true), m_count(indices.size()) {

            const size_t attribs_size = sizeof(VertexData<Attribs...>);

            glGenVertexArrays(1, &m_vertex_array_id);
            glGenBuffers(1, &m_vertex_buffer_id);
            glGenBuffers(1, &m_element_buffer_id);

            glBindVertexArray(m_vertex_array_id);

            glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_id);
            glBufferData(GL_ARRAY_BUFFER, attribs_size * vertices.size(), vertices.data(),
                         GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_element_buffer_id);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(), indices.data(), GL_STATIC_DRAW);

            auto temp = std::tuple<Attribs...>();
            std::apply([&](auto &&...args) { AddVertexAttrib(attribs_size, 0, args...); }, temp);
            m_stride = sizeof(VertexData<Attribs...>);

            EnableAttribs();
            glBindVertexArray(0);
            DisableAttribs();
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }

        template<typename ...Attribs>
        VertexArray(const std::shared_ptr<Context> &ctx, const std::vector<VertexData<Attribs...>> &vertices)
                :ContextObject(ctx), m_is_elements(false), m_count(vertices.size()) {
            const size_t attribs_size = sizeof(VertexData<Attribs...>);

            glGenVertexArrays(1, &m_vertex_array_id);
            glGenBuffers(1, &m_vertex_buffer_id);

            glBindVertexArray(m_vertex_array_id);

            glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_id);
            glBufferData(GL_ARRAY_BUFFER, attribs_size * vertices.size(), vertices.data(),
                         GL_STATIC_DRAW);

            auto temp = std::tuple<Attribs...>();
            std::apply([&](auto &&...args) { AddVertexAttrib(attribs_size, 0, args...); }, temp);
            m_stride = sizeof(VertexData<Attribs...>);

            EnableAttribs();
            glBindVertexArray(0);
            DisableAttribs();
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }

    private:
        // Program can call Draw(mode)
        friend class Program;

        template<typename T>
        GLenum GetGlType();

        void AddVertexAttrib(int total, int offset) {
            LIT_ASSERT(offset == total, "Vertex data should not contain paddings", m_ctx->GetLogger())
        }

        template<typename T, int SIZE, typename ...Attribs>
        void AddVertexAttrib(int total, int offset, glm::vec<SIZE, T> attrib, Attribs... attribs) {
            m_attribs.push_back({SIZE, GetGlType<T>(), offset});
            AddVertexAttrib(offset + sizeof(T) * SIZE, attribs...);
        }

        void EnableAttribs();

        void DisableAttribs();

        void Draw(DrawMode mode);

        struct VertexAttrib {
            int size;
            GLenum type;
            int offset;
        };

        size_t m_count = 0;

        bool m_is_elements = false;

        std::vector<VertexAttrib> m_attribs;
        uint32_t m_stride;

        GLuint m_vertex_array_id;
        GLuint m_vertex_buffer_id;
        GLuint m_element_buffer_id;
    };
}