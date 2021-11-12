#pragma once

#include <glm/vec3.hpp>
#include <lit/common/glm_ext/comparators.hpp>
#include <set>
#include <optional>
#include <memory>
#include <lit/engine/algorithms/allocator.hpp>
#include <functional>

namespace lit::engine {

    class VoxelWorld {
    public:

        using ChunkIndexType = uint32_t;
        using VoxelType = uint32_t;

        inline static const int WORLD_SIZE_LOG = 9;
        inline static const int CHUNK_SIZE_LOG = 5;
        inline static const int GRID_LOD_NUM = WORLD_SIZE_LOG - CHUNK_SIZE_LOG + 1;
        inline static const int CHUNK_LOD_NUM = CHUNK_SIZE_LOG + 1;
        inline static const int CHUNK_SIZE = (1 << CHUNK_SIZE_LOG);

        static_assert(CHUNK_SIZE_LOG < 10);
        static_assert(WORLD_SIZE_LOG < 10);

        using ChunkRaw = std::array<std::array<std::array<VoxelType, CHUNK_SIZE>, CHUNK_SIZE>, CHUNK_SIZE>;
        using ChunkWithLods = std::array<VoxelType, 0x49249249u & ~((~0u) << (3 * CHUNK_SIZE_LOG))>;

        inline static const ChunkIndexType CHUNK_UNKNOWN = 0xFFFF'FFFFu;
        inline static const ChunkIndexType CHUNK_EMPTY = 0x0000'0000u;

        constexpr static glm::ivec3 GetDims() { return glm::ivec3{8192, 1024, 8192}; }

        constexpr static size_t GetGridWithLodsSize() {
            size_t res = 0;
            for (int i = 0; i < GRID_LOD_NUM; i++) res += glm::compMul(GetChunkGridDims() >> i);
            return res;
        }

        constexpr static glm::ivec3 GetChunkDims() { return glm::ivec3{1 << CHUNK_SIZE_LOG}; }

        constexpr static glm::ivec3 GetChunkGridDims() { return GetDims() >> CHUNK_SIZE_LOG; }

        VoxelWorld(std::function<void(glm::ivec3 chunk_position, ChunkRaw &chunkRaw)> chunk_generator);

        void SetVoxel(int x, int y, int z, uint32_t value);

        void SetVoxel(glm::ivec3 pos, uint32_t value);

        uint32_t GetVoxel(int x, int y, int z) const;

        uint32_t GetVoxel(glm::ivec3 pos, uint32_t value) const;

        uint64_t GetVersion() const;

    private:

        void ResetChunk(glm::ivec3 )

        bool IsKnownChunk(glm::ivec3 chunk_pos) const;

        bool IsValidChunk(glm::ivec3 chunk_pos) const;

        std::optional<glm::ivec3> GetNextUnknownChunk();

        uint64_t m_version = 0;
        std::vector<uint32_t> m_index_pool;

        std::set<glm::ivec3, lit::common::glm_ext::vec3_comparator<float>> m_requests;

        std::vector<ChunkWithLods> m_chunks;
        std::vector<ChunkIndexType> m_grid_data_with_lods;
    };

}