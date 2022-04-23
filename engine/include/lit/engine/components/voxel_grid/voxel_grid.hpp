#pragma once
#include "voxel_grid_dense.hpp"
#include "voxel_grid_sparse.hpp"

namespace lit::engine {

    using VoxelGridBase = VoxelGridBaseT<uint32_t>;
    using VoxelGridDense = VoxelGridDenseT<uint32_t>;
    using VoxelGridSparse = VoxelGridSparseT<uint32_t>;

    using VoxelGridPtr = std::shared_ptr<VoxelGridBase>;

}