#pragma once

#include <lit/engine/components/voxel_grid/voxel_grid_sparse.hpp>

namespace lit::engine {
    class VoxelWorldGenerator {
    public:
        static VoxelGridSparseT<uint32_t> Generate();
    };
}