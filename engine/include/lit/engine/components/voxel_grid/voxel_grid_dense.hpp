#pragma once

#include <lit/common/glm_ext/region.hpp>
#include <lit/common/images/images.hpp>

namespace lit::engine {

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

    class VoxelGrid {
    public:
        static const int kMaxLodNum = 10;
        static const int kHighestLodSizeThreshold = 4;
        static const int kMinSize = 1;
        static const int kMaxSize = 1024;

        /**
         * Create empty grid (all values are zero) with a given dimensions.
         * Use @binary=true if you want to store only 1 bit per voxel.
         * By default you can store uint8_t value per voxel.
         */
        explicit VoxelGrid(glm::ivec3 dims = glm::ivec3(1), bool binary = false);

        /**
         * Create empty grid (all values are zero) with a given dimensions and anchor.
         * Use @binary=true if you want to store only 1 bit per voxel.
         * By default you can store uint8_t value per voxel.
         */
        VoxelGrid(glm::ivec3 dims, glm::dvec3 anchor, bool binary = false);

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

        int GetHighestLod() const;

        glm::dvec3 GetAnchor() const;

        friend class VoxelRenderer;

    private:
        int GetLodDataBit(glm::ivec3 pos) const;

        bool HasVoxel(glm::ivec3 pos, int lod) const;

        void SetHasVoxel(glm::ivec3 pos, int lod, bool value);

        lit::common::Image3D<uint8_t> m_data;
        lit::common::Image3D<uint8_t> m_lod_data[kMaxLodNum]; // bit-compressed

        glm::ivec3 m_dims = {1, 1, 1};

        bool m_binary = false;

        glm::dvec3 m_anchor;

        std::vector<std::pair<glm::ivec3, uint8_t>> m_deferred_set_voxel;
    };

}