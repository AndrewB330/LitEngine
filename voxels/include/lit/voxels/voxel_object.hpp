#pragma once

#include "lit/voxels/voxel_grid.hpp"
#include <lit/common/glm_ext/transform.hpp>

namespace lit::voxels {

    using lit::common::glm_ext::transform3;

    struct TransformChanges {
        bool num_changes;
        transform3 previous_transform;
        transform3 current_transform;

        TransformChanges(int num, transform3 prev, transform3 cur)
                : num_changes(num), previous_transform(prev), current_transform(cur) {}
    };

    struct VoxelObjectChanges {
        VoxelGridChanges grid_changes;
        TransformChanges transform_changes;

        VoxelObjectChanges(VoxelGridChanges g, TransformChanges t)
                : grid_changes(std::move(g)), transform_changes(t) {}
    };

    class VoxelObject : public VoxelGrid {
    public:

        VoxelObject(transform3 transform, glm::ivec3 dims, bool binary = false);

        VoxelObject(transform3 transform, glm::vec3 center, glm::ivec3 dims, bool binary = false);

        ~VoxelObject() override = default;

        void TransformDeferred(glm::vec3 translate, glm::dquat rotate, double scale);

        /// Apply both object and grid changes
        VoxelObjectChanges ApplyAllDeferredChanges();

        const transform3 & GetTransform() const;

        const glm::vec3 & GetCenter() const;

        glm::mat4 GetGlobalModelMat() const;

        glm::mat4 GetGlobalModelMatInv() const;

    private:
        glm::vec3 center_m;
        transform3 transform_m;

        std::vector<transform3> deferred_transforms_m;
    };

    void ReadFromFile(VoxelObject & object, const std::string & filename);
}
