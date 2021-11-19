#pragma once

#include <lit/engine/components/voxel_world.hpp>

namespace lit::engine {
    class VoxelWorldGenerator {
    public:
        static VoxelWorld Generate();
    };
}