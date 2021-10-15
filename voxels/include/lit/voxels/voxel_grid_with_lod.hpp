#pragma once

#include <vector>

#include <glm/vec3.hpp>
#include <lit/common/glm_ext/region.hpp>
#include <lit/common/images/image.hpp>
#include "voxel_grid_2.hpp"

namespace lit::voxels {

    const int K_MAX_LOD_NUM = 7; // lod 0 (original data, compressed) ... lod 6 (64*64*64 lod data)

    struct VoxelGridDeferredChanges {
        int voxels_changed = 0;
        std::vector<iregion3> changed_regions;
        std::vector<iregion3> changed_lod_regions[K_MAX_LOD_NUM];
    };

    template<typename T>
    class VoxelGridWithLodT {
    public:
        using VoxelGridType = VoxelGridT<T>;
        using VoxelGridLodType = VoxelGridBool;

        explicit VoxelGridWithLodT(glm::ivec3 dimensions, T fillValue = VoxelGridType::kDefaultValue)
                : m_dimensions(dimensions) {
            if (!IsBoolean()) {
                m_grid = VoxelGridLodType(dimensions, fillValue);
            }

            int highest_lod = GetHighestLod();
            for (int lod = 0; lod < highest_lod; lod++) {
                m_lod_grid[lod] = VoxelGridLodType(GetDimensionsForLod(lod), fillValue);
            }
        }

        glm::ivec3 GetDimensions() const {
            return m_dimensions;
        }

        glm::ivec3 GetDimensionsForLod(int lod) const {
            return glm::max(m_dimensions >> lod, 1);
        }

        bool GetVoxel(int x, int y, int z) const {
            return IsBoolean() ? m_lod_grid[0].GetVoxel(x, y, z) : m_grid.GetVoxel(x, y, z);
        }

        bool GetVoxel(glm::ivec3 pos) const {
            return IsBoolean() ? m_lod_grid[0].GetVoxel(pos) : m_grid.GetVoxel(pos);
        }

        void SetVoxelDeferred(int x, int y, int z, T value) {
            deferred_set_voxel_m.emplace_back(glm::ivec3(x, y, z), value);
        }

        void SetVoxelDeferred(glm::ivec3 pos, T value) {
            deferred_set_voxel_m.emplace_back(pos, value);
        }

        VoxelGridDeferredChanges ApplyDeferredChanges() {
            if (deferred_set_voxel_m.empty()) {
                return VoxelGridChanges();
            }
            iregion3 changed_region = iregion3::empty();
            for (const auto &[pos, value] : deferred_set_voxel_m) {
                changed_region.populate(pos);
                if (!IsBoolean())
                    m_data.set_pixel(pos, value);
                m_lod_grid[0].SetVoxel(pos, value > 0);
            }

            VoxelGridDeferredChanges changes;

            changes.voxels_changed = static_cast<int>(deferred_set_voxel_m.size());
            changes.changed_regions.push_back(changed_region);
            changes.changed_lod_regions[0].push_back(changed_region);

            changed_region = changed_region.scaled_down();

            int highest_lod = GetHighestLod();
            for (int lod = 1; lod < highest_lod; lod++) {
                iregion3 next_changed_region = iregion3::empty();
                bool changed = false;

                for (int x = changed_region.begin.x; x < changed_region.end.x; x++) {
                    for (int y = changed_region.begin.y; y < changed_region.end.y; y++) {
                        for (int z = changed_region.begin.z; z < changed_region.end.z; z++) {
                            bool old_value = m_lod_grid[lod].GetVoxel(x, y, z);
                            bool new_value = m_lod_data[lod - 1].GetVoxelBlock(x, y, z);
                            if (old_value != new_value) {
                                changed = true;
                                next_changed_region.populate(glm::ivec3(x, y, z));
                                m_lod_grid[lod].SetVoxel(x, y, z, new_value);
                            }
                        }
                    }
                }

                if (!changed)
                    break;

                changes.changed_lod_regions[lod].push_back(next_changed_region);
                changed_region = next_changed_region.scaled_down();
            }

            deferred_set_voxel_m.clear();

            return changes;
        }

        bool IsBoolean() const {
            return std::is_same<T, bool>::value;
        }

        int GetHighestLod() const {
            // todo: is this logic ok?
            for (int lod = 1; lod < kLodNum; lod++) {
                if ((m_dims.x >> lod) < 8 || (m_dims.y >> lod) < 8 || (m_dims.z >> lod) < 8)
                    return lod;
            }
            return kLodNum;
        }

    private:

        glm::ivec3 m_dimensions;
        VoxelGridType m_grid;
        VoxelGridLodType m_lod_grid[K_MAX_LOD_NUM];

        std::vector<std::pair<glm::ivec3, T>> deferred_set_voxel_m;
    };

}