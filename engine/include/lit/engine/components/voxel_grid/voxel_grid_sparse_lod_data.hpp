#pragma once

#include <lit/engine/components/voxel_grid/voxel_grid_sparse.hpp>

namespace lit::engine {

    template<typename VoxelType>
    struct VoxelGridSparseLodData {
        static const size_t DATA_SIZE = 0x49249249u & ~((~0u) << (3 * VoxelGridSparseT<VoxelType>::CHUNK_SIZE_LOG + 1));
        static const size_t DATA_SIZE_WITHOUT_ZERO_LOD = DATA_SIZE >> 3;
        static const size_t BINARY_DATA_SIZE = (DATA_SIZE + sizeof(BinaryDataType) - 1) / sizeof(BinaryDataType);

        using BinaryDataType = uint32_t;
        using ChunkLodData = std::array<VoxelType, DATA_SIZE_WITHOUT_ZERO_LOD>;
        using ChunkBinaryLodData = std::array<BinaryDataType, BINARY_DATA_SIZE>;

        VoxelGridSparseLodData() {

        }

        std::vector<typename VoxelGridSparseT<VoxelType>::ChunkIndexType> m_grid_lod_data;
        std::vector<ChunkBinaryLodData> m_chunk_binary_lod_data;
        std::vector<> m_chunk_lod_data;
    };

}