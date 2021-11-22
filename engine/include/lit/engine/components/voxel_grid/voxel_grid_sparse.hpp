#pragma once

#include <lit/common/glm_ext/comparators.hpp>
#include <lit/engine/algorithms/allocator.hpp>
#include <lit/engine/components/voxel_grid/voxel_grid_base.hpp>
#include <glm/vec3.hpp>
#include <unordered_set>
#include <functional>
#include <memory>
#include <array>
#include <deque>

#define PARALLEL_GENERATION

namespace lit::engine {

    /// <summary>
    /// Implementation of abstract component VoxelGridBase, use it when your voxel grid has a lot of empty voxels.
    /// Internally it divides grid into chunks of size <see cref="VoxelGridSparseT::CHUNK_SIZE"/> and stores only non-empty chunks.
    /// Use only dimensions that are multiple of <see cref="VoxelGridSparseT::CHUNK_SIZE"/>.
    /// </summary>
    /// <typeparam name="T">Type of voxel data</typeparam>
    template<typename VoxelType>
    class VoxelGridSparseT : public VoxelGridBaseT<VoxelType> {
    public:

        /// <summary>
        /// Log2 of chunk side size. Number of bits needed to represent coordinates inside chunk.
        /// </summary>
        inline static const int CHUNK_SIZE_LOG = 5;
        /// <summary>
        /// Chunk side size. Chunk has CHUNK_SIZE x CHUNK_SIZE x CHUNK_SIZE dimensions.
        /// </summary>
        inline static const int CHUNK_SIZE = (1 << CHUNK_SIZE_LOG);

        static_assert(CHUNK_SIZE_LOG <= 10);

        /// <summary>
        /// Type that stores voxel data for a single chunk. Data is represented as (CHUNK_SIZE^3) consecutive values.
        /// </summary>
        using ChunkData = std::array<std::array<std::array<VoxelType, CHUNK_SIZE>, CHUNK_SIZE>, CHUNK_SIZE>;

        /// <summary>
        /// Each chunk has a unique (except index 0) index that allows to identify it.
        /// After chunk is destroyed its index may be reused for another chunk.
        /// </summary>
        using ChunkIndexType = uint32_t;
        /// <summary>
        /// Chunk with index 0 is guaranteed to be empty. This index can represent many chunks at once.
        /// </summary>
        inline static const ChunkIndexType CHUNK_EMPTY = 0x0000'0000u;

        constexpr static glm::ivec3 GetChunkDimensions() { return glm::ivec3(CHUNK_SIZE); }

        glm::ivec3 GetChunkGridDimensions() const { return VoxelGridBaseT<VoxelType>::GetDimensions() >> CHUNK_SIZE_LOG; }

        VoxelGridSparseT(const glm::ivec3& dimensions, const glm::dvec3& anchor)
            :VoxelGridBaseT<VoxelType>(((glm::clamp(dimensions, 1, 1 << 15) - 1) | (CHUNK_SIZE - 1) + 1), anchor) {}

        void SetVoxel(const glm::ivec3& position, VoxelType value) override {
            glm::ivec3 chunk_grid_position = position >> CHUNK_SIZE_LOG;
            if (!IsValidChunk(chunk_grid_position)) {
                return;
            }

            ChunkIndexType chunk_index = m_chunk_grid.GetPixel(chunk_grid_position);
            if (chunk_index == 0 && value == 0) {
                // If chunk is empty and value is 0 we can skip.
                return;
            }

            if (chunk_index == 0) {
                // Chunk is empty! We should create a new one.
                chunk_index = CreateChunk();
            }

            auto& chunk = m_chunks[chunk_index];
            glm::ivec3 relative_position = position & (CHUNK_SIZE - 1);

            if (chunk[relative_position.x][relative_position.y][relative_position.z] == value) {
                // Value was already there, nothin changed.
                return;
            }

            chunk[relative_position.x][relative_position.y][relative_position.z] = value;
            InvokeOnVoxelChangedCallbacks(position, value);
            InvokeOnChunkAnyChangeCallbacks(chunk_index, ChunkChangeType::Changed, position, chunk_grid_position, relative_position);
        }

        VoxelType GetVoxel(const glm::ivec3& position) const override {
            glm::ivec3 chunk_grid_position = position >> CHUNK_SIZE_LOG;
            if (!IsValidChunk(chunk_grid_position)) {
                return 0;
            }

            ChunkIndexType chunk_index = m_chunk_grid.GetPixel(chunk_grid_position);
            if (chunk_index == 0) {
                return 0;
            }

            auto& chunk = m_chunks[chunk_index];
            glm::ivec3 relative_position = position & (CHUNK_SIZE - 1);
            return chunk[relative_position.x][relative_position.y][relative_position.z];
        };

        void WriteGridDataTo(ChunkIndexType* destination) const {
            memcpy(destination, m_chunk_grid.GetDataPointer(), glm::compMul(GetChunkGridDimensions()) * sizeof(ChunkIndexType));
        }

        void WriteChunkDataTo(VoxelType* destination, uint32_t* dest_compressed, ChunkIndexType index, int min_lod = 0);

        enum class ChunkChangeType {
            Created,
            Changed,
            Deleted
        };

        using OnChunkAnyChangeCallback =
            std::function<void(
                ChunkIndexType index,
                ChunkChangeType change,
                const glm::ivec3& global_position,
                const glm::ivec3& chunk_grid_position,
                const glm::ivec3& relative_position)>;

        size_t AddOnChunkAnyChangeCallback(OnChunkAnyChangeCallback callback) {
            m_chunk_callbacks.emplace_back(std::move(callback));
            return m_chunk_callbacks.size() - 1;
        }

        void RemoveOnChunkAnyChangeCallback(size_t index) {
            OnChunkAnyChangeCallback().swap(m_chunk_callbacks[index]);
        }

        size_t GetSizeBytes() const override {
            return VoxelGridBaseT<VoxelType>::GetSizeBytes() - sizeof(VoxelGridBaseT<VoxelType>) +
                sizeof(VoxelGridSparseT<VoxelType>) +
                m_chunk_callbacks.capacity() * sizeof(OnChunkAnyChangeCallback) +
                m_chunk_index_allocator.GetSizeBytes() - sizeof(FixedAllocator) +
                m_chunks.size() * sizeof(ChunkData) +
                m_positions.capacity() * sizeof(glm::ivec3) +
                glm::compMul(m_chunk_grid.GetDimensions()) * sizeof(ChunkIndexType);
        }

    private:

        bool IsEmptyChunk(glm::ivec3 chunk_grid_position) const {
            return m_chunk_grid.GetPixel(chunk_grid_position);
        }

        bool IsValidChunk(glm::ivec3 chunk_grid_position) const {
            return glm::all(glm::greaterThanEqual(chunk_grid_position, glm::ivec3(0))) &&
                glm::all(glm::lessThan(chunk_grid_position, GetChunkGridDimensions()));
        }

        void InvokeOnChunkAnyChangeCallbacks(ChunkIndexType index,
                                             ChunkChangeType change,
                                             const glm::ivec3& global_position,
                                             const glm::ivec3& chunk_grid_position,
                                             const glm::ivec3& relative_position) {
            for (auto& callback : m_chunk_callbacks) {
                if (callback) {
                    callback(index, change, global_position, chunk_grid_position, relative_position);
                }
            }
        }

        // Important: There is no check if chunk was already created!
        ChunkIndexType CreateChunk(const glm::ivec3& chunk_grid_position) {
            ChunkIndexType index = m_chunk_index_allocator.Allocate();
            if (index >= m_chunks.size()) {
                m_chunks.emplace_back();
                m_positions.emplace_back(chunk_grid_position);
            }
            InvokeOnChunkAnyChangeCallbacks(index, ChunkChangeType::Created, glm::ivec3(), chunk_grid_position, glm::ivec3());
            return index;
        }

        std::vector<OnChunkAnyChangeCallback> m_chunk_callbacks;

        FixedAllocator m_chunk_index_allocator = FixedAllocator(0);

        std::deque<ChunkData> m_chunks;
        std::vector<glm::ivec3> m_positions;
        lit::common::Image3D<ChunkIndexType> m_chunk_grid;
    };

}