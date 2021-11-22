#pragma once
#include <lit/common/glm_ext/region.hpp>
#include <lit/engine/algorithms/fenwick_tree.hpp>

namespace lit::engine {

    inline static const glm::ivec3 INVALID_TEXTURE_ADDRESS = glm::ivec3(-1, -1, -1);

    class Allocator3D {
    public:
        explicit Allocator3D(glm::ivec3 size);

        glm::ivec3 Allocate(glm::ivec3 size);

        void Free(glm::ivec3 address, glm::ivec3 size);

    private:
        FenwickTreeRange3D m_storage;
        glm::ivec3 m_size;
    };

    class FixedAllocator {
    public:
        explicit FixedAllocator(uint32_t n = 0);

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

    class FixedAllocator3D {
    public:
        FixedAllocator3D(glm::ivec3 size, glm::ivec3 cell);

        glm::ivec3 Allocate();

        void Free(glm::ivec3 address);

    private:
        uint32_t Convert(glm::ivec3 address);

        glm::ivec3 Convert(uint32_t address);

        glm::ivec3 m_grid;
        glm::ivec3 m_cell;

        uint32_t m_address;
        uint32_t m_max_address;
        std::vector<uint32_t> m_address_pool;
    };

}