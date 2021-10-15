#pragma once

#include <cstdint>
#include <lit/gl/context.hpp>
#include "lit/voxels/voxel_world.hpp"
#include "camera.hpp"

namespace lit::voxels {

    class VoxelsPipeline {
    public:
        explicit VoxelsPipeline(const std::shared_ptr<gl::Context> & ctx);

        void Run(Camera &camera, VoxelWorld& world);

    private:
        class VoxelsPipelineImpl;

        struct VoxelsPipelineImplDeleter {
            void operator()(VoxelsPipelineImpl * obj);
        };

        std::unique_ptr<VoxelsPipelineImpl, VoxelsPipelineImplDeleter> m_impl;
    };

}