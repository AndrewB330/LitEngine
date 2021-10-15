#include "voxel_data_manager.hpp"
#include "lit/voxels/voxel_grid.hpp"

using namespace lit::voxels;

VoxelDataManager &VoxelDataManager::Instance() {
    static VoxelDataManager instance;
    return instance;
}

void VoxelDataManager::VoxelGridCreated(const VoxelGrid &voxel_grid) {
    if (m_lod_textures.find(voxel_grid.GetId()) != m_lod_textures.end()) {
        return;
    }

    {
        lit::gl::Texture3DInfo info;
        info.data_format = lit::gl::TextureDataFormat::RedInteger;
        info.data_type = lit::gl::TextureDataType::UnsignedByte;
        info.internal_format = lit::gl::TextureInternalFormat::R8UI;

        info.width = voxel_grid.GetDimsLod(0).x;
        info.height = voxel_grid.GetDimsLod(0).y;
        info.depth = voxel_grid.GetDimsLod(0).z;
        info.mip_levels = voxel_grid.GetHighestLod() - 1;

        auto texture = std::make_shared<lit::gl::Texture3D>(m_ctx, info);
        m_lod_textures[voxel_grid.GetId()] = texture;

        for (int i = 0; i <= info.mip_levels; i++) {
            texture->Update(voxel_grid.m_lod_data[i], i);
        }
    }

    if (!voxel_grid.IsBinary()) {
        lit::gl::Texture3DInfo data_info;
        data_info.data_format = lit::gl::TextureDataFormat::RedInteger;
        data_info.data_type = lit::gl::TextureDataType::UnsignedByte;
        data_info.internal_format = lit::gl::TextureInternalFormat::R8UI;
        data_info.width = voxel_grid.GetDims().x;
        data_info.height = voxel_grid.GetDims().y;
        data_info.depth = voxel_grid.GetDims().z;

        auto data_texture = std::make_shared<lit::gl::Texture3D>(m_ctx, data_info);
        m_data_textures[voxel_grid.GetId()] = data_texture;
        data_texture->Update(voxel_grid.m_data, 0);
    }
}

void VoxelDataManager::VoxelGridDeleted(const VoxelGrid &voxel_grid) {
    if (m_lod_textures.find(voxel_grid.GetId()) == m_lod_textures.end()) {
        return;
    }

    m_lod_textures.erase(voxel_grid.GetId());
    if (!voxel_grid.IsBinary()) {
        m_data_textures.erase(voxel_grid.GetId());
    }
}

void VoxelDataManager::VoxelGridChanged(const VoxelGrid &voxel_grid, const VoxelGridChanges &changes) {
    if (changes.num_changes == 0 || m_lod_textures.find(voxel_grid.GetId()) == m_lod_textures.end()) {
        return;
    }

    for (int lod = 0; lod < changes.affected_lod_regions.size(); lod++) {
        auto texture = m_lod_textures[voxel_grid.GetId()];
        auto region = changes.affected_lod_regions[lod];

        region.begin = region.begin - region.begin % kMinSide;
        region.end = region.end + (kMinSide - region.end % kMinSide) % kMinSide;

        region = region.clamped(iregion3(glm::ivec3(0), voxel_grid.GetDimsLod(lod)));

        texture->Update(voxel_grid.m_lod_data[lod], region, lod);
    }


    if (!voxel_grid.IsBinary()) {
        auto texture = m_data_textures[voxel_grid.GetId()];

        auto region = changes.affected_region;

        region.begin = region.begin - region.begin % kMinSide;
        region.end = region.end + (kMinSide - region.end % kMinSide) % kMinSide;

        region = region.clamped(iregion3(glm::ivec3(0), voxel_grid.GetDims()));

        texture->Update(voxel_grid.m_data, region, 0);
    }
}

std::shared_ptr<VoxelDataManager::Texture> VoxelDataManager::GetDataTexture(int id) {
    return m_data_textures[id];
}

std::shared_ptr<VoxelDataManager::Texture> VoxelDataManager::GetLodTexture(int id) {
    return m_lod_textures[id];
}

void VoxelDataManager::SetContext(const std::shared_ptr<gl::Context> &ctx) {
    m_ctx = ctx;
}
