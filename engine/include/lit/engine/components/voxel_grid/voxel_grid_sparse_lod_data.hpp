#pragma once

#include <lit/engine/components/voxel_grid/voxel_grid_sparse.hpp>

namespace lit::engine {

    inline size_t GetLodTotalSize(const glm::ivec3& base_size, int first_lod, int last_lod) {
        glm::ivec3 size = base_size >> first_lod;
        size_t total = 0;
        for (int i = first_lod; i <= last_lod && size != glm::ivec3(0); i++) {
            total += ((size_t)size.x) * ((size_t)size.y) * ((size_t)size.z);
            size = (size + 1) >> 1;
        }
        return total;
    }

    template<typename VoxelType>
    struct VoxelGridSparseLodDataT {
        using VoxelGrid = VoxelGridSparseT<VoxelType>;
        using ChunkIndexType = VoxelGrid::ChunkIndexType;

        VoxelGridSparseLodDataT() {}

        void SetDimensions(glm::ivec3 chunk_grid_dimensions) {
            m_max_grid_lod = 0;
            glm::ivec3 d = chunk_grid_dimensions;
            while (glm::compMul((d + 1) >> 1) > 1) {
                d = (d + 1) >> 1;
                m_max_grid_lod++;
            }
            m_total_lod = m_max_grid_lod + VoxelGrid::CHUNK_SIZE_LOG;
            m_chunk_grid_dimensions = chunk_grid_dimensions;
            m_grid_lod_data.assign(GetLodTotalSize(chunk_grid_dimensions, 0, m_max_grid_lod), VoxelGrid::CHUNK_EMPTY);
        }

        Array3DView<ChunkIndexType> GetChunkGridViewAtLod(int lod) {
            return Array3DView(m_grid_lod_data.begin() + GetLodTotalSize(m_chunk_grid_dimensions, 0, lod - 1), m_chunk_grid_dimensions >> lod);
        }

        Array3DView<VoxelType> GetChunkViewAtLod(ChunkIndexType index, int lod) {
            size_t offset = (index * GetLodTotalSize(VoxelGrid::GetChunkDimensions(), 1, VoxelGrid::CHUNK_SIZE_LOG)) +
                GetLodTotalSize(VoxelGrid::GetChunkDimensions(), 1, lod - 1);
            return Array3DView(m_grid_lod_data.begin() + offset, VoxelGrid::GetChunkDimensions() >> lod);
        }

        Array3DViewBool<uint32_t> GetBinaryChunkAtLod(ChunkIndexType index, int lod) {
            size_t offset = (index * GetLodTotalSize(VoxelGrid::GetChunkDimensions(), 0, VoxelGrid::CHUNK_SIZE_LOG)) +
                GetLodTotalSize(VoxelGrid::GetChunkDimensions(), 0, lod - 1);
            return Array3DViewBool(m_chunk_binary_lod_data.begin(), VoxelGrid::GetChunkDimensions() >> lod, offset);
        }

        std::vector<ChunkIndexType> m_grid_lod_data;
        std::vector<VoxelType> m_chunk_lod_data;
        std::vector<uint32_t> m_chunk_binary_lod_data;
        glm::ivec3 m_chunk_grid_dimensions;
        int m_total_lod = 0;
        int m_max_grid_lod = 0;
    };

}