#include <glm/common.hpp>
#include "lit/engine/components/voxel_grid.hpp"

using namespace lit::engine;

VoxelGrid::VoxelGrid(glm::ivec3 dims, bool binary) : VoxelGrid(dims, glm::dvec3(dims) / 2.0, binary) {}

VoxelGrid::VoxelGrid(glm::ivec3 dims, glm::dvec3 anchor, bool binary)
        : m_dims(dims), m_binary(binary), m_anchor(anchor) {
    m_dims = glm::clamp(m_dims, kMinSize, kMaxSize);

    if (!binary) {
        m_data = lit::common::Image3D<uint8_t>(m_dims.x, m_dims.y, m_dims.z);
    }

    int highest_lod = GetHighestLod();
    for (int lod = 0; lod < highest_lod; lod++) {
        auto lod_dims = GetDimsLod(lod);
        m_lod_data[lod] = lit::common::Image3D<uint8_t>(lod_dims.x, lod_dims.y, lod_dims.z);
    }
}

glm::ivec3 VoxelGrid::GetDims() const {
    return m_dims;
}

glm::ivec3 VoxelGrid::GetDimsLod(int lod) const {
    return (m_dims + (1 << (lod))) >> (lod + 1);
}

void VoxelGrid::SetVoxelDeferred(glm::ivec3 pos, uint8_t value) {
    if (pos.x < 0 || pos.y < 0 || pos.z < 0 || pos.x >= m_dims.x || pos.y >= m_dims.y || pos.z >= m_dims.z) return;
    m_deferred_set_voxel.emplace_back(pos, value);
}

void VoxelGrid::SetVoxelDeferred(int x, int y, int z, uint8_t value) {
    SetVoxelDeferred(glm::ivec3(x, y, z), value);
}

uint8_t VoxelGrid::GetVoxel(glm::ivec3 pos) const {
    if (pos.x < 0 || pos.y < 0 || pos.z < 0 || pos.x >= m_dims.x || pos.y >= m_dims.y || pos.z >= m_dims.z) return 0;
    if (m_binary) {
        return HasVoxel(pos, 0);
    }
    return m_data.GetPixel(pos);
}

uint8_t VoxelGrid::GetVoxel(int x, int y, int z) const {
    return GetVoxel(glm::ivec3(x, y, z));
}

int VoxelGrid::GetLodDataBit(glm::ivec3 pos) const {
    return (pos.x & 1) | ((pos.y & 1) << 1) | ((pos.z & 1) << 2);
}

bool VoxelGrid::IsBinary() const {
    return m_binary;
}

VoxelGridChanges VoxelGrid::ApplyGridChanges() {
    if (m_deferred_set_voxel.empty()) {
        return {};
    }
    iregion3 changed_region = iregion3::empty();
    for (const auto &[pos, value]: m_deferred_set_voxel) {
        changed_region.populate(pos);
        if (!m_binary)
            m_data.SetPixel(pos, value);
        SetHasVoxel(pos, 0, value > 0);
    }

    VoxelGridChanges changes;

    changes.num_changes = static_cast<int>(m_deferred_set_voxel.size());
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
                    bool new_value = m_lod_data[lod - 1].GetPixel(x, y, z) > 0;
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

    m_deferred_set_voxel.clear();

    return changes;
}

int VoxelGrid::GetHighestLod() const {
    for (int lod = 1; lod < kMaxLodNum; lod++) {
        auto lod_dims = GetDimsLod(lod);
        if (lod_dims.x < kHighestLodSizeThreshold &&
            lod_dims.y < kHighestLodSizeThreshold &&
            lod_dims.z < kHighestLodSizeThreshold)
            return lod;
    }
    return kMaxLodNum;
}

bool VoxelGrid::HasVoxel(glm::ivec3 pos, int lod) const {
    return (m_lod_data[lod].GetPixel(pos >> 1) >> GetLodDataBit(pos)) & 1;
}

void VoxelGrid::SetHasVoxel(glm::ivec3 pos, int lod, bool value) {
    int bit = GetLodDataBit(pos);
    m_lod_data[lod].GetPixel(pos >> 1) &= ~(1u << bit);
    m_lod_data[lod].GetPixel(pos >> 1) |= (value << bit);
}

glm::dvec3 VoxelGrid::GetAnchor() const {
    return m_anchor;
}
