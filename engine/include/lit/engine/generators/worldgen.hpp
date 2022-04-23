#pragma once

#include <lit/engine/components/voxel_grid/voxel_grid_base.hpp>

#include <lit/common/random.hpp>
#include <spdlog/spdlog.h>

namespace lit::engine {
    class WorldGen {
    public:
        WorldGen() = default;

        void Generate(lit::engine::VoxelGridBaseT<uint32_t> &world, spdlog::logger &logger);

        void ResetTestWorld(lit::engine::VoxelGridBaseT<uint32_t> &world);

        void
        PlaceObject(lit::engine::VoxelGridBaseT<uint32_t> &world, const lit::engine::VoxelGridBaseT<uint32_t> &object);

        std::shared_ptr<lit::engine::VoxelGridBaseT<uint32_t>> GenerateTree(RandomGen & rng);

    private:
    };
}
