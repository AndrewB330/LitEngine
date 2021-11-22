#pragma once
#include <lit/engine/components/voxel_grid/voxel_grid_sparse.hpp>
#include <lit/engine/algorithms/allocator.hpp>
#include <lit/rendering/opengl/texture.hpp>
#include <lit/rendering/opengl/uniform_buffer.hpp>
#include <unordered_set>
#include <array>
#include <map>
#include <thread>

namespace lit::engine {
    using namespace lit::rendering::opengl;

    class VoxelWorldGpuDataManager {
    public:
        VoxelWorldGpuDataManager();

        void Update(VoxelGridSparseT<uint32_t> & world, glm::dvec3 position);

        UniformBuffer & GetWorldDataBuffer();

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

        static inline const uint64_t WORLD_BUFFER_SIZE_BYTES = 1024ll * 1024ll * 128; // 128MB
        static inline const uint64_t CHUNK_BUFFER_SIZE_BYTES = 1024ll * 1024ll * 1024ll * 3; // 3GB
        static inline const uint64_t INFO_BUFFER_SIZE_BYTES = 1024ll * 1024ll * 2; // 2MB

        static inline const int BUCKET_NUM = 3;
        static inline const int BUCKET_SIZE_BYTES[] = {
                CHUNK_BUFFER_SIZE_BYTES / 2,
                CHUNK_BUFFER_SIZE_BYTES / 4,
                CHUNK_BUFFER_SIZE_BYTES / 4,
        };
        static_assert(sizeof(BUCKET_SIZE_BYTES)/sizeof(BUCKET_SIZE_BYTES[0]) == BUCKET_NUM);

        struct ChunkInfo {
            uint32_t global_address;
            uint32_t bucket;
        };

        // World
        uint64_t m_prev_world_version = 0;
        UniformBuffer m_world_data_buffer;
        // Chunks
        std::vector<uint32_t> m_chunk_bucket;
        std::vector<uint32_t> m_chunk_address; // relative address, inside bucket
        UniformBuffer m_chunk_data_buffer;
        UniformBuffer m_chunk_info_buffer;
        UniformBuffer m_chunk_compressed_data_buffer;

        FixedAllocator m_allocator[BUCKET_NUM];

        bool m_world_registered = false;

        std::vector<uint32_t> m_sorted_chunk_indices;
        const uint32_t UPDATES_PER_FRAME = 30;
        uint32_t m_current_i = 0;
        uint32_t m_current_bucket = 0;
        uint32_t m_chunks_in_current_bucket = 0;
    };
}