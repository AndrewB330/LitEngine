#pragma once

#include <memory>

namespace lit::rendering::opengl {

    class UniformBuffer {
    public:
        static UniformBuffer Create(uint64_t size, void * data = nullptr);

        UniformBuffer(UniformBuffer&&) = default;
        UniformBuffer& operator=(UniformBuffer&&) = default;

        ~UniformBuffer();

        void Bind(int index);

        void * GetHostPtr();

    public:
        explicit UniformBuffer(uint64_t size, void * data);

        uint64_t m_size;
        void * m_host_ptr;
        std::unique_ptr<uint32_t> m_buffer_id;
    };

}