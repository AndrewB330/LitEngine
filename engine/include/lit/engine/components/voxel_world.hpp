#pragma once

#include <lit/common/glm_ext/comparators.hpp>
#include <lit/engine/algorithms/allocator.hpp>
#include <glm/vec3.hpp>
#include <unordered_set>
#include <functional>
#include <memory>
#include <array>
#include <deque>

#define PARALLEL_GENERATION

namespace lit::engine {

    class VoxelWorld {
    public:

        using ChunkIndexType = uint32_t;
        using VoxelType = uint32_t;

        inline static const int WORLD_SIZE_LOG = 8;
        inline static const int CHUNK_SIZE_LOG = 5;
        inline static const int GRID_LOD_NUM = WORLD_SIZE_LOG - CHUNK_SIZE_LOG + 1;
        inline static const int CHUNK_LOD_NUM = CHUNK_SIZE_LOG + 1;
        inline static const int CHUNK_SIZE = (1 << CHUNK_SIZE_LOG);

        static_assert(CHUNK_SIZE_LOG <= 10);
        static_assert(WORLD_SIZE_LOG <= 10);

        using ChunkRaw = std::array<std::array<std::array<VoxelType, CHUNK_SIZE>, CHUNK_SIZE>, CHUNK_SIZE>;
        using ChunkWithLods = std::array<VoxelType, 0x49249249u & ~((~0u) << (3 * CHUNK_SIZE_LOG + 1))>;

        inline static const ChunkIndexType CHUNK_UNKNOWN = 0xFFFF'FFFFu;
        inline static const ChunkIndexType CHUNK_EMPTY = 0x0000'0000u;

        constexpr static glm::ivec3 GetDims() { return glm::ivec3{512, 256, 512}; }

        constexpr static size_t GetGridWithLodsSize() {
            size_t res = 0;
            for (int i = 0; i < GRID_LOD_NUM; i++) res += glm::compMul(GetChunkGridDims() >> i);
            return res;
        }

        constexpr static glm::ivec3 GetChunkDims() { return glm::ivec3{1 << CHUNK_SIZE_LOG}; }

        constexpr static glm::ivec3 GetChunkGridDims() { return GetDims() >> CHUNK_SIZE_LOG; }

        VoxelWorld();

        void SetGenerator(std::function<void(glm::ivec3, ChunkRaw &)> chunk_generator);

        void SetVoxel(glm::ivec3 pos, uint32_t value);

        VoxelType GetVoxel(glm::ivec3 pos) const;

        glm::ivec3 GetChunkCenterByIndex(ChunkIndexType index) const;

        uint64_t GetVersion() const;

        void WriteGridDataTo(ChunkIndexType *dest);

        void WriteChunkDataTo(VoxelType *dest, ChunkIndexType index, int min_lod = 0);

        void SetChunkCreatedCallback(std::function<void(ChunkIndexType)> callback) const;

        void SetChunkChangedCallback(std::function<void(ChunkIndexType)> callback) const;

        void SetChunkDeletedCallback(std::function<void(ChunkIndexType)> callback) const;

        ChunkIndexType GetChunksNum() const;

        size_t GetSize() const;

    private:

        void Generate(glm::ivec3 start_grid_position);

        void UpdateGrid();

        void UpdateChunk(ChunkIndexType index);

        void SetChunk(glm::ivec3 grid_position, ChunkIndexType chunk_index, std::unordered_set<glm::ivec3> & queue);

        void SetEmpty(glm::ivec3 grid_position, std::unordered_set<glm::ivec3> & queue);

        size_t GridPosToIndex(glm::ivec3 grid_position, int lod = 0) const;

        size_t ChunkPosToIndex(glm::ivec3 position, int lod = 0) const;

        bool IsKnownChunk(glm::ivec3 grid_position) const;

        bool IsValidChunk(glm::ivec3 grid_position) const;

        std::function<void(glm::ivec3, ChunkRaw &)> m_chunk_generator;
        mutable std::function<void(ChunkIndexType index)> m_chunk_created;
        mutable std::function<void(ChunkIndexType index)> m_chunk_changed;
        mutable std::function<void(ChunkIndexType index)> m_chunk_deleted;

        uint64_t m_grid_changes = 0;
        uint64_t m_version = 0;

        FixedAllocator m_chunk_index_allocator{1'000'000};

        std::deque<ChunkWithLods> m_chunks;
        std::vector<ChunkIndexType> m_grid_data_with_lods;
        std::vector<glm::ivec3> m_positions;
    };

}