#pragma once

#include <map>
#include <GL/glew.h>

#include <lit/common/algorithms/fenwick_tree.hpp>
#include <lit/common/glm_ext/comparators.hpp>
#include <lit/gl/texture.hpp>
#include <memory>

namespace lit::voxels {

    class VoxelGrid;

    class VoxelGridChanges;

    class VoxelDataManager {
    public:

        using Texture = lit::gl::Texture3D;

        constexpr static const glm::ivec3 kDataDims = glm::ivec3(256, 256, 256);
        constexpr static const int kMinSide = 4;

        static VoxelDataManager &Instance();

        void VoxelGridCreated(const VoxelGrid &voxel_grid);

        void VoxelGridDeleted(const VoxelGrid &voxel_grid);

        void VoxelGridChanged(const VoxelGrid &voxel_grid, const VoxelGridChanges &changes);

        std::shared_ptr<Texture> GetDataTexture(int id);

        std::shared_ptr<Texture> GetLodTexture(int id);

        void SetContext(const std::shared_ptr<gl::Context> & ctx);

    private:
        VoxelDataManager() = default;

        std::shared_ptr<gl::Context> m_ctx;

        std::map<int, std::shared_ptr<Texture>> m_data_textures;
        std::map<int, std::shared_ptr<Texture>> m_lod_textures;
    };

}
