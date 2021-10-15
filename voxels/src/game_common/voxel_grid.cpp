#include "lit/voxels/voxel_grid.hpp"
#include <glm/gtx/extended_min_max.hpp>
#include "../gpu_data_managers/voxel_data_manager.hpp"

using namespace lit::voxels;
/*
int id_counter_g = 1;

VoxelGrid::VoxelGrid(glm::ivec3 dims, bool binary)
        : m_dims(dims), binary_m(binary), id_m(id_counter_g++) {
    m_dims = glm::clamp(m_dims, 1, 1024);

    if (!binary) {
        m_data = lit::common::image3d<uint8_t>(m_dims.x, m_dims.y, m_dims.z);
    }

    int highest_lod = GetHighestLod();
    for (int lod = 0; lod < highest_lod; lod++) {
        auto lod_dims = GetDimsLod(lod);
        m_lod_data[lod] = lit::common::image3d<uint8_t>(lod_dims.x, lod_dims.y, lod_dims.z);
    }

    VoxelDataManager::Instance().VoxelGridCreated(*this);
}

glm::ivec3 VoxelGrid::GetDims() const {
    return m_dims;
}

glm::ivec3 VoxelGrid::GetDimsLod(int lod) const {
    return glm::max(m_dims >> (lod + 1), 1);
}

void VoxelGrid::SetVoxelDeferred(glm::ivec3 pos, uint8_t value) {
    if (pos.x < 0 || pos.y < 0 || pos.z < 0 || pos.x >= m_dims.x || pos.y >= m_dims.y || pos.z >= m_dims.z) return;
    deferred_set_voxel_m.emplace_back(pos, value);
}

VoxelGrid::~VoxelGrid() {
    VoxelDataManager::Instance().VoxelGridDeleted(*this);
}

void VoxelGrid::SetVoxelDeferred(int x, int y, int z, uint8_t value) {
    SetVoxelDeferred(glm::ivec3(x, y, z), value);
}

uint8_t VoxelGrid::GetVoxel(glm::ivec3 pos) const {
    if (pos.x < 0 || pos.y < 0 || pos.z < 0 || pos.x >= m_dims.x || pos.y >= m_dims.y || pos.z >= m_dims.z) return 0;
    if (binary_m) {
        return HasVoxel(pos, 0);
    }
    return m_data.get_pixel(pos);
}

uint8_t VoxelGrid::GetVoxel(int x, int y, int z) const {
    return GetVoxel(glm::ivec3(x, y, z));
}

int VoxelGrid::GetLodDataBit(glm::ivec3 pos) const {
    return (pos.x & 1) | ((pos.y & 1) << 1) | ((pos.z & 1) << 2);
}

bool VoxelGrid::IsBinary() const {
    return binary_m;
}

int VoxelGrid::GetId() const {
    return id_m;
}


std::vector<uint8_t> VoxelGrid::GetDataRegion(iregion3 region) const {
    if (binary_m)
        return {};

    region = region.clamped(iregion3(glm::ivec3(), m_dims));

    std::vector<uint8_t> data;
    data.reserve(region.size());

    for (int z = region.begin.z; z < region.end.z; z++) {
        for (int y = region.begin.y; y < region.end.y; y++) {
            for (int x = region.begin.x; x < region.end.x; x++) {
                data.push_back(voxel_data_m[GetDataIndex(glm::ivec3(x, y, z))]);
            }
        }
    }

    return data;
}

std::vector<uint8_t> VoxelGrid::GetLodDataRegion(int lod, iregion3 region) const {
    region = region.clamped(iregion3(glm::ivec3(), GetDimsLod(lod)));

    std::vector<uint8_t> data;
    data.reserve(region.size());

    for (int z = region.begin.z; z < region.end.z; z++) {
        for (int y = region.begin.y; y < region.end.y; y++) {
            for (int x = region.begin.x; x < region.end.x; x++) {
                data.push_back(lod_data_m[lod][GetLodDataIndex(glm::ivec3(x, y, z) << 1, lod)]);
            }
        }
    }

    return data;
}

VoxelGridChanges VoxelGrid::ApplyGridChanges() {
    if (deferred_set_voxel_m.empty()) {
        return VoxelGridChanges();
    }
    iregion3 changed_region = iregion3::empty();
    for (const auto &[pos, value] : deferred_set_voxel_m) {
        changed_region.populate(pos);
        if (!binary_m)
            m_data.set_pixel(pos, value);
        SetHasVoxel(pos, 0, value > 0);
    }

    VoxelGridChanges changes;

    changes.num_changes = static_cast<int>(deferred_set_voxel_m.size());
    changes.affected_region = changed_region;

    changed_region = changed_region.scaled_down();
    changes.affected_lod_regions.push_back(changed_region);

    int highest_lod = GetHighestLod();
    for (int lod = 1; lod < highest_lod; lod++) {
        iregion3 next_changed_region = iregion3::empty();
        bool changed = false;

        for (int x = changed_region.begin.x; x < changed_region.end.x; x++) {
            for (int y = changed_region.begin.y; y < changed_region.end.y; y++) {
                for (int z = changed_region.begin.z; z < changed_region.end.z; z++) {
                    bool old_value = HasVoxel(glm::ivec3(x, y, z), lod);
                    bool new_value = m_lod_data[lod - 1].get_pixel(x, y, z) > 0;
                    if (old_value != new_value) {
                        changed = true;
                        next_changed_region.populate(glm::ivec3(x, y, z));
                        SetHasVoxel(glm::ivec3(x, y, z), lod, new_value);
                    }
                }
            }
        }

        if (!changed)
            break;

        changed_region = next_changed_region.scaled_down();
        changes.affected_lod_regions.push_back(changed_region);
    }

    deferred_set_voxel_m.clear();

    VoxelDataManager::Instance().VoxelGridChanged(*this, changes);

    return changes;
}

int VoxelGrid::GetHighestLod() const {
    for (int lod = 1; lod < kLodNum; lod++) {
        if ((m_dims.x >> lod) < 8 || (m_dims.y >> lod) < 8 || (m_dims.z >> lod) < 8)
            return lod;
    }
    return kLodNum;
}

bool VoxelGrid::HasVoxel(glm::ivec3 pos, int lod) const {
    return (m_lod_data[lod].get_pixel(pos >> 1) >> GetLodDataBit(pos)) & 1;
}

void VoxelGrid::SetHasVoxel(glm::ivec3 pos, int lod, bool value) {
    int bit = GetLodDataBit(pos);
    m_lod_data[lod].get_pixel(pos >> 1) &= ~(1u << bit);
    m_lod_data[lod].get_pixel(pos >> 1) |= (value << bit);
}
*/