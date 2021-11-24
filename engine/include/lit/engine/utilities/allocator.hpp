#pragma once
#include <vector>

namespace lit::engine {

    class ContiguousAllocator {
    public:
        explicit ContiguousAllocator(uint32_t n = 0);

        uint32_t Allocate();

        bool CanAllocate();

        void Free(uint32_t address);

        uint32_t GetSize() const;

        uint32_t GetPtr() const;

        size_t GetSizeBytes() const;

    private:
        uint32_t m_size;
        uint32_t m_ptr = 0;
        std::vector<uint32_t> m_pool;
    };

}