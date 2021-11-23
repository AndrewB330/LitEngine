#pragma once

#include <lit/engine/components/voxel_grid/voxel_grid_sparse.hpp>

namespace lit::engine {

    size_t GetLodTotalSize(const glm::ivec3& base_size, int first_lod, int last_lod = 32) {
        glm::ivec3 size = base_size >> first_lod;
        size_t total = 0;
        for (int i = first_lod; i <= last_lod && size != glm::ivec3(0); i++) {
            total += ((size_t)size.x) * ((size_t)size.y) * ((size_t)size.z);
            size >>= 1;
        }
        return total;
    }

    template<typename VoxelType>
    struct VoxelGridSparseLodDataT {
        using CompressedDataType = uint32_t;

        static const size_t DATA_SIZE = 0x49249249u & ~((~0u) << (3 * VoxelGridSparseT<VoxelType>::CHUNK_SIZE_LOG + 1));
        static const size_t DATA_SIZE_WITHOUT_ZERO_LOD = DATA_SIZE >> 3;
        static const size_t COMPRESSED_DATA_SIZE = (DATA_SIZE + sizeof(CompressedDataType) - 1) / sizeof(CompressedDataType);

        using ChunkLodData = std::array<VoxelType, DATA_SIZE_WITHOUT_ZERO_LOD>;
        using ChunkCompressedLodData = std::array<CompressedDataType, COMPRESSED_DATA_SIZE>;

        VoxelGridSparseLodDataT() {

        }

        std::vector<typename VoxelGridSparseT<VoxelType>::ChunkIndexType> m_grid_lod_data;
        std::vector<ChunkCompressedLodData> m_chunk_binary_lod_data;
        std::vector<ChunkLodData> m_chunk_lod_data;
    };

}