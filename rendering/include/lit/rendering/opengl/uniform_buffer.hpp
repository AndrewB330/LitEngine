#pragma once

#include <memory>

namespace lit::rendering::opengl {

    enum UniformBufferAccess {
        Write,
        Read,
        ReadAndWrite
    };

    struct UniformBufferInfo {
        uint64_t size = 1;
        UniformBufferAccess access = UniformBufferAccess::Write;
        bool flushable = false;
        bool coherent = false;
        void * data = nullptr;
    };

    class UniformBuffer {
    public:
        static UniformBuffer Create(UniformBufferInfo buffer_info);

        UniformBuffer(UniformBuffer&&) = default;
        UniformBuffer& operator=(UniformBuffer&&) = default;

        ~UniformBuffer();

        void Bind(int index);

        void * GetHostPtr();

        template<typename T>
        T* GetHostPtrAs() {
            return (T*) GetHostPtr();
        }

    public:
        explicit UniformBuffer(UniformBufferInfo buffer_info);

        uint64_t m_size;
        void * m_host_ptr;
        std::unique_ptr<uint32_t> m_buffer_id;
    };

}