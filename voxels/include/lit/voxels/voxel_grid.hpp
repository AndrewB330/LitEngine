#pragma once

#include <vector>

#include <glm/vec3.hpp>
#include <lit/common/glm_ext/region.hpp>
#include <lit/common/images/image.hpp>

namespace lit::voxels {

    using lit::common::glm_ext::iregion3;

    /**
     * Describes region of voxels that was changed.
     * @num_changes - number of voxels that were changed
     * @affected_region - bounding box of region of where changes were made.
     * @affected_lod_regions - list of regions for different lod levels.
     */
    struct VoxelGridChanges {
        int num_changes = 0;
        iregion3 affected_region = iregion3::empty();
        std::vector<iregion3> affected_lod_regions = {}; // affected regions for each lod level
    };

    /**
     * 3D grid with 8bit data stored in each cell (voxel).
     * Maintains different levels of details in compressed format.
     * LODs require 30% extra memory on top of data size.
     *
     * Each VoxelGrid has unique id, so it can not be copied, only cloned with new id.
     */
    class VoxelGrid {
    public:

        static const int kLodNum = 7; // lod 0 (original data, compressed) ... lod 6 (64*64*64 lod data)

        /**
         * Create empty grid (all values are zero) with a given dimensions.
         * Use @binary=true if you want to store only 1 bit per voxel.
         * By default you can store uint8_t value per voxel.
         */
        explicit VoxelGrid(glm::ivec3 dims, bool binary = false);

        VoxelGrid(const VoxelGrid &) = delete;

        virtual ~VoxelGrid();

        /**
         * Set value of @pos voxel to @value after next ApplyGridChanges is called.
         * If several calls were made then only last @value will be considered.
         */
        void SetVoxelDeferred(glm::ivec3 pos, uint8_t value);

        /**
         * Set value of (@x, @y, @z) voxel to @value after next ApplyGridChanges is called.
         * If several calls were made then only last @value will be considered.
         */
        void SetVoxelDeferred(int x, int y, int z, uint8_t value);

        /**
         * Apply all deferred grid changes (SetVoxelDeferred) and return changes summary.
         * Todo: Return many aggregated regions instead of only one bounding box.
         * Todo: For example if we changed only two opposite corner voxels
         * Todo: we should not report entire grid update, only two corners.
         */
        VoxelGridChanges ApplyGridChanges();

        uint8_t GetVoxel(glm::ivec3 pos) const;

        uint8_t GetVoxel(int x, int y, int z) const;

        glm::ivec3 GetDims() const;

        glm::ivec3 GetDimsLod(int lod) const;

        bool IsBinary() const;

        int GetId() const;

        int GetHighestLod() const;

        friend class VoxelDataManager;

    private:

        int GetLodDataBit(glm::ivec3 pos) const;

        bool HasVoxel(glm::ivec3 pos, int lod) const;

        void SetHasVoxel(glm::ivec3 pos, int lod, bool value);

        glm::ivec3 m_dims = {1, 1, 1};

        lit::common::image3d<uint8_t> m_data;
        lit::common::image3d<uint8_t> m_lod_data[kLodNum]; // bit-compressed

        std::vector<std::pair<glm::ivec3, uint8_t>> deferred_set_voxel_m;

        bool binary_m = false;

        int id_m;
    };

}
