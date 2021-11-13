#pragma once

#include <glm/glm.hpp>
#include "api.hpp"

namespace lit::rendering::opengl {

    template<typename ...Attribs>
    struct VertexData {
        std::tuple<Attribs...> attribs;
    };

    enum class DrawMode {
        Points,
        LineStrip,
        LineLoop,
        Lines,
        Triangles,
        TriangleStrip,
        TriangleFan,
    };

    class VertexArray {
    public:
        template<typename ...Attribs>
        VertexArray(const std::vector<VertexData<Attribs...>> &vertices,
                    const std::vector<uint32_t> &indices)
                : m_is_elements(true), m_count(indices.size()) {
            auto temp = std::tuple<Attribs...>();
            std::apply([&](auto &&...args) { AddVertexAttrib(sizeof(VertexData<Attribs...>), 0, args...); }, temp);
            m_stride = sizeof(VertexData<Attribs...>);

            CreateGlVertexArrayWithIndices((void*)vertices.data(), sizeof(VertexData<Attribs...>) * vertices.size(),
                                           (void*)indices.data(), sizeof(int) * indices.size());
        }

        template<typename ...Attribs>
        VertexArray(const std::vector<VertexData<Attribs...>> &vertices)
                :m_is_elements(false), m_count(vertices.size()) {
            auto temp = std::tuple<Attribs...>();
            std::apply([&](auto &&...args) { AddVertexAttrib(sizeof(VertexData<Attribs...>), 0, args...); }, temp);
            m_stride = sizeof(VertexData<Attribs...>);

            CreateGlVertexArray((void*)vertices.data(), sizeof(VertexData<Attribs...>) * vertices.size());
        }

    private:
        // Shader can call Draw(mode)
        friend class Shader;

        template<typename T>
        uint32_t GetGlType();

        void CreateGlVertexArray(void *vertex_data, uint32_t vertex_data_size);

        void CreateGlVertexArrayWithIndices(void *vertex_data, uint32_t vertex_data_size, void *index_data,
                                            uint32_t index_data_size);

        template<typename T, int SIZE>
        void AddVertexAttrib(int total, int offset, glm::vec<SIZE, T> attrib) {
            m_attribs.push_back({SIZE, GetGlType<T>(), offset});
            LIT_ASSERT(offset + sizeof(T) * SIZE == total, "Vertex data should not contain paddings", spdlog::default_logger())
        }

        template<typename T, int SIZE, typename ...Attribs>
        void AddVertexAttrib(int total, int offset, glm::vec<SIZE, T> attrib, Attribs... attribs) {
            m_attribs.push_back({SIZE, GetGlType<T>(), offset});
            AddVertexAttrib(total, offset + sizeof(T) * SIZE, attribs...);
        }

        void EnableAttribs();

        void DisableAttribs();

        void Draw(DrawMode mode);

        struct VertexAttrib {
            int size;
            uint32_t type;
            int offset;
        };

        size_t m_count = 0;

        bool m_is_elements = false;

        std::vector<VertexAttrib> m_attribs;
        uint32_t m_stride;

        uint32_t m_vertex_array_id;
        uint32_t m_vertex_buffer_id;
        uint32_t m_element_buffer_id;
    };
}