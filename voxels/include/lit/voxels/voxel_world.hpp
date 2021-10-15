#pragma once

#include <vector>
#include <memory>

#include <lit/common/glm_ext/transform.hpp>
#include <lit/common/glm_ext/region.hpp>

#include "lit/voxels/voxel_object.hpp"

namespace lit::voxels {

    using lit::common::glm_ext::transform3;
    using lit::common::glm_ext::iregion3;

    class VoxelWorld {
    public:

        explicit VoxelWorld(glm::ivec3 shadow_dims, float shadow_scale);

        void AddObject(std::shared_ptr<VoxelObject> object_to_add);

        const std::vector<std::shared_ptr<VoxelObject>> & GetObjects() const;

        void Init();

        void Update();

        // todo: remove
        std::shared_ptr<VoxelObject> GetShadow() const {
            return m_shadow_object;
        }

    private:
        void ApplyAllDeferredChanges();

        void RasterizeShadow(const VoxelObject & object, transform3 transform, iregion3 region, bool clear);

        std::vector<std::shared_ptr<VoxelObject>> m_objects;
        std::shared_ptr<VoxelObject> m_shadow_object;
    };

}
