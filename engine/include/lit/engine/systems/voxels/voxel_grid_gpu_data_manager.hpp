#pragma once
#include <lit/engine/components/voxel_grid/voxel_grid_sparse.hpp>
#include <lit/engine/systems/voxels/voxel_grid_lod_manager.hpp>
#include <lit/engine/utilities/allocator.hpp>
#include <lit/rendering/opengl/texture.hpp>
#include <lit/rendering/opengl/uniform_buffer.hpp>
#include <lit/engine/systems/system.hpp>
#include <unordered_set>
#include <array>
#include <map>
#include <thread>

namespace lit::engine {
    using namespace lit::rendering::opengl;

    using VoxelGrid = VoxelGridSparseT<uint32_t>;
    using VoxelGridLod = VoxelGridSparseLodDataT<uint32_t>;
    using ChunkCreatedArgs = VoxelGrid::ChunkCreatedArgs;
    using ChunkChangedArgs = VoxelGrid::ChunkChangedArgs;
    using ChunkDeletedArgs = VoxelGrid::ChunkDeletedArgs;
    using ChunkAnyChangeArgs = std::variant<ChunkCreatedArgs, ChunkChangedArgs, ChunkDeletedArgs>;

    class VoxelGridGpuDataManager : public System {
    public:
        VoxelGridGpuDataManager(entt::registry& registry, VoxelGridLodManager<uint32_t> & lod_manager);

        void CommitChanges(glm::dvec3 observer_position);

        UniformBuffer & GetChunkGridDataBuffer();

        UniformBuffer& GetChunkDataBuffer();

        UniformBuffer& GetChunkCompressedDataBuffer();

        UniformBuffer & GetChunkInfoBuffer();

        uint64_t GetWorldLodOffsetDword(int lod) const;

        uint64_t GetWorldLodSizeDword(int lod) const;

        uint64_t GetChunkLodOffsetDword(int bucket, int lod) const;

        uint64_t GetBucketOffsetDword(uint32_t bucket) const;

        uint64_t GetChunkLodSizeDword(int lod) const;

        uint64_t GetChunkSizeDword(int bucket) const;

    private:

        void RegisterNewEntities();

        void ProcessAllChangesForEntity(entt::entity ent, const std::vector<ChunkAnyChangeArgs> & changes);

        uint32_t GetGlobalAddress(uint32_t index) const;

        static inline const uint64_t MEGABYTE = 1024ll * 1024ll;
        static inline const uint64_t GIGABYTE = MEGABYTE * 1024ll;

        static inline const uint64_t CHUNK_GRID_BUFFER_SIZE_BYTES = 128 * MEGABYTE;
        static inline const uint64_t CHUNK_BUFFER_SIZE_BYTES = 2 * GIGABYTE;
        static inline const uint64_t CHUNK_BIT_BUFFER_SIZE_BYTES = 2 * GIGABYTE;
        static inline const uint64_t INFO_BUFFER_SIZE_BYTES = 2 * MEGABYTE;

        static inline const int BUCKET_NUM = 3;
        static inline const int BUCKET_SIZE_BYTES[] = {
                CHUNK_BUFFER_SIZE_BYTES / 2,
                CHUNK_BUFFER_SIZE_BYTES / 4,
                CHUNK_BUFFER_SIZE_BYTES / 4,
        };
        static_assert(sizeof(BUCKET_SIZE_BYTES)/sizeof(BUCKET_SIZE_BYTES[0]) == BUCKET_NUM);

        struct ChunkInfo {
            uint32_t global_data_address;
            uint32_t bucket;
        }; 
        
        VoxelGridLodManager<uint32_t>& m_lod_manager;

        bool m_registered = false;

        // Chunk grid
        UniformBuffer m_chunk_grid_data_buffer;

        // Chunks
        std::vector<uint32_t> m_chunk_bucket;
        std::vector<uint32_t> m_chunk_address; // relative address, inside bucket
        UniformBuffer m_chunk_data_buffer;
        UniformBuffer m_chunk_info_buffer;
        UniformBuffer m_chunk_bit_data_buffer;

        ContiguousAllocator m_allocator[BUCKET_NUM];

        std::vector<uint32_t> m_sorted_chunk_indices;
        const uint32_t UPDATES_PER_FRAME = 30;
        uint32_t m_current_i = 0;
        uint32_t m_current_bucket = 0;
        uint32_t m_chunks_in_current_bucket = 0;

        std::unordered_map<entt::entity, std::vector<ChunkAnyChangeArgs>> m_changes;

        std::unordered_map<entt::entity, size_t> m_callback_handle;
    };
}