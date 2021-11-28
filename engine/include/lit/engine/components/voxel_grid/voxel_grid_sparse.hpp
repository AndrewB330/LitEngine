#pragma once

#include <lit/common/glm_ext/comparators.hpp>
#include <lit/engine/utilities/allocator.hpp>
#include <lit/engine/utilities/array_view.hpp>
#include <lit/engine/components/voxel_grid/voxel_grid_base.hpp>
#include <glm/vec3.hpp>
#include <variant>
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

        glm::ivec3 GetChunkGridDimensions() const {
            return VoxelGridBaseT<VoxelType>::GetDimensions() >> CHUNK_SIZE_LOG;
        }

        VoxelGridSparseT(const glm::ivec3& dimensions, const glm::dvec3& anchor)
            : VoxelGridBaseT<VoxelType>((((glm::clamp(dimensions, 1, 1 << 15) - 1) | (CHUNK_SIZE - 1)) + 1), anchor),
            m_chunk_grid_data(glm::compMul(VoxelGridBaseT<VoxelType>::m_dimensions >> CHUNK_SIZE_LOG), CHUNK_EMPTY),
            m_chunk_grid(VoxelGridBaseT<VoxelType>::m_dimensions >> CHUNK_SIZE_LOG, m_chunk_grid_data.data()) {
            // Create fake zero chunk
            CreateChunk({ 0, 0, 0 });
        }

        void SetVoxel(const glm::ivec3& position, VoxelType value) override {
            glm::ivec3 chunk_grid_position = position >> CHUNK_SIZE_LOG;
            if (!IsValidChunk(chunk_grid_position)) {
                return;
            }

            ChunkIndexType chunk_index = m_chunk_grid.At(chunk_grid_position);
            if (chunk_index == 0 && value == 0) {
                // If chunk is empty and value is 0 we can skip.
                return;
            }

            if (chunk_index == 0) {
                // Chunk is empty! We should create a new one.
                chunk_index = CreateChunk(chunk_grid_position);
            }

            auto& chunk = m_chunks[chunk_index];
            glm::ivec3 relative_position = position & (CHUNK_SIZE - 1);

            if (chunk[relative_position.x][relative_position.y][relative_position.z] == value) {
                // Value was already there, nothin changed.
                return;
            }

            chunk[relative_position.x][relative_position.y][relative_position.z] = value;
            VoxelGridBaseT<VoxelType>::InvokeOnVoxelChangedCallbacks(position, value);
            InvokeOnChunkAnyChangeCallbacks(ChunkChangedArgs{
                chunk_index,
                chunk_grid_position,
                position,
                relative_position,
                value });
        }

        VoxelType GetVoxel(const glm::ivec3& position) const override {
            glm::ivec3 chunk_grid_position = position >> CHUNK_SIZE_LOG;
            if (!IsValidChunk(chunk_grid_position)) {
                return 0;
            }

            ChunkIndexType chunk_index = m_chunk_grid.At(chunk_grid_position);
            if (chunk_index == 0) {
                return 0;
            }

            auto& chunk = m_chunks[chunk_index];
            glm::ivec3 relative_position = position & (CHUNK_SIZE - 1);
            return chunk[relative_position.x][relative_position.y][relative_position.z];
        };

        /*void WriteGridDataTo(ChunkIndexType* destination) const {
            memcpy(destination, m_chunk_grid.Data(),
                glm::compMul(GetChunkGridDimensions()) * sizeof(ChunkIndexType));
        }

        void WriteChunkDataTo(VoxelType* destination, ChunkIndexType index) {
            memcpy(destination, (void*)&m_chunks[index], sizeof(ChunkData));
        }*/

        struct ChunkCreatedArgs {
            ChunkIndexType index;
            glm::ivec3 chunk_grid_position;
        };

        struct ChunkChangedArgs {
            ChunkIndexType index;
            glm::ivec3 chunk_grid_position;
            glm::ivec3 global_position;
            glm::ivec3 relative_position;
            VoxelType value;
        };

        struct ChunkDeletedArgs {
            ChunkIndexType index;
            glm::ivec3 chunk_grid_position;
        };

        friend class ChunkView;

        using OnChunkAnyChangeCallback =
            std::function<void(std::variant<ChunkCreatedArgs, ChunkChangedArgs, ChunkDeletedArgs>)>;

        size_t AddOnChunkAnyChangeCallback(OnChunkAnyChangeCallback callback) {
            m_chunk_callbacks.emplace_back(std::move(callback));
            return m_chunk_callbacks.size() - 1;
        }

        glm::ivec3 GetChunkGridPos(ChunkIndexType index) {
            return m_positions[index];
        }

        void RemoveOnChunkAnyChangeCallback(size_t index) {
            OnChunkAnyChangeCallback().swap(m_chunk_callbacks[index]);
        }

        class ChunkView {
        public:
            void SetVoxel(const glm::ivec3& relative_position, VoxelType value) {
                auto& chunk = m_owner.m_chunks[m_index];

                if (chunk[relative_position.x][relative_position.y][relative_position.z] == value) {
                    // Value was already there, nothin changed.
                    return;
                }

                chunk[relative_position.x][relative_position.y][relative_position.z] = value;

                glm::ivec3 chunk_grid_position = GetChunkGridPosition();
                glm::ivec3 position = (chunk_grid_position << CHUNK_SIZE_LOG) + relative_position;
                m_owner.InvokeOnVoxelChangedCallbacks(position, value);
                m_owner.InvokeOnChunkAnyChangeCallbacks(ChunkChangedArgs{
                    m_index,
                    chunk_grid_position,
                    position,
                    relative_position,
                    value });
            }

            VoxelType GetVoxel(const glm::ivec3& relative_position) const {
                return m_owner.m_chunks[m_index][relative_position.x][relative_position.y][relative_position.z];
            }

            glm::ivec3 GetChunkGridPosition() const {
                return m_owner.m_positions[m_index];
            }

            ChunkIndexType GetIndex() const {
                return m_index;
            }

        private:
            friend class VoxelGridSparseT<VoxelType>;

            ChunkView(ChunkIndexType index, VoxelGridSparseT<VoxelType>& owner) :m_index(index), m_owner(owner) {}

            ChunkIndexType m_index;
            VoxelGridSparseT<VoxelType>& m_owner;
        };

        void InvokeForAllChunks(std::function<void(const ChunkView&)> function) {
            auto grid_dims = GetChunkGridDimensions();
            for (int i = 0; i < grid_dims.x; i++) {
                for (int j = 0; j < grid_dims.y; j++) {
                    for (int k = 0; k < grid_dims.z; k++) {
                        if (m_chunk_grid.At(i, j, k)) {
                            function(ChunkView(m_chunk_grid.At(i, j, k), *this));
                        }
                    }
                }
            }
        }

        size_t GetSizeBytes() const override {
            return VoxelGridBaseT<VoxelType>::GetSizeBytes() - sizeof(VoxelGridBaseT<VoxelType>) +
                sizeof(VoxelGridSparseT<VoxelType>) +
                m_chunk_callbacks.capacity() * sizeof(OnChunkAnyChangeCallback) +
                m_chunk_index_allocator.GetSizeBytes() - sizeof(ContiguousAllocator) +
                m_chunks.size() * sizeof(ChunkData) +
                m_positions.capacity() * sizeof(glm::ivec3) +
                m_chunk_grid_data.capacity() * sizeof(ChunkIndexType);
        }

        const Array3DView<ChunkIndexType>& GetChunkGridView() const {
            return m_chunk_grid;
        }

        const Array3DView<VoxelType> GetChunkViewAsArray(ChunkIndexType index) const {
            return Array3DView<VoxelType>(GetChunkDimensions(), (VoxelType*)m_chunks[index].data());
        }

    private:

        bool IsEmptyChunk(glm::ivec3 chunk_grid_position) const {
            return m_chunk_grid.GetPixel(chunk_grid_position);
        }

        bool IsValidChunk(glm::ivec3 chunk_grid_position) const {
            return glm::all(glm::greaterThanEqual(chunk_grid_position, glm::ivec3(0))) &&
                glm::all(glm::lessThan(chunk_grid_position, GetChunkGridDimensions()));
        }

        void InvokeOnChunkAnyChangeCallbacks(const std::variant<ChunkCreatedArgs, ChunkChangedArgs, ChunkDeletedArgs>& args) {
            for (auto& callback : m_chunk_callbacks) {
                if (callback) {
                    callback(args);
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
            m_chunk_grid.At(chunk_grid_position) = index;
            InvokeOnChunkAnyChangeCallbacks(ChunkCreatedArgs{ index, chunk_grid_position });
            return index;
        }

        std::vector<OnChunkAnyChangeCallback> m_chunk_callbacks;

        ContiguousAllocator m_chunk_index_allocator = ContiguousAllocator(0);

        std::deque<ChunkData> m_chunks;
        std::vector<glm::ivec3> m_positions;
        std::vector<ChunkIndexType> m_chunk_grid_data;
        Array3DView<ChunkIndexType> m_chunk_grid;
    };

}