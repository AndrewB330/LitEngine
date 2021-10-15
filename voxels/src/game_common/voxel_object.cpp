#include "lit/voxels/voxel_object.hpp"
#include <fstream>

using namespace lit::voxels;


VoxelObject::VoxelObject(transform3 transform, glm::vec3 center, glm::ivec3 dims, bool binary)
        : center_m(center), transform_m(transform), VoxelGrid(dims, binary) {}

VoxelObject::VoxelObject(transform3 transform, glm::ivec3 dims, bool binary)
        : VoxelObject(transform, glm::vec3(dims) / 2.f, dims, binary) {}

void VoxelObject::TransformDeferred(glm::vec3 translate, glm::dquat rotate, double scale) {
    deferred_transforms_m.emplace_back(translate, rotate, scale);
}

VoxelObjectChanges VoxelObject::ApplyAllDeferredChanges() {
    transform3 old_transform = transform_m;
    int num_changes = static_cast<int>(deferred_transforms_m.size());

    for (auto t : deferred_transforms_m) {
        transform_m.translation += t.translation;
        transform_m.rotation = glm::normalize(t.rotation * transform_m.rotation);
        transform_m.scale *= t.scale;
    }

    deferred_transforms_m.clear();

    auto grid_changes = ApplyGridChanges();

    return VoxelObjectChanges(grid_changes, TransformChanges(num_changes, old_transform, transform_m));
}

const transform3 &VoxelObject::GetTransform() const {
    return transform_m;
}

const glm::vec3 &VoxelObject::GetCenter() const {
    return center_m;
}

glm::mat4 VoxelObject::GetGlobalModelMat() const {
    return transform_m.mat() * glm::translate(-center_m);
}

glm::mat4 VoxelObject::GetGlobalModelMatInv() const {
    return glm::translate(center_m) * transform_m.mat_inv();
}

void lit::voxels::ReadFromFile(VoxelObject &object, const std::string &filename) {
    std::ifstream in(filename);
    auto dims = object.GetDims();
    for (int i = 0; i < dims.y; i++) {
        for (int j = 0; j < dims.x; j++) {
            for (int k = 0; k < dims.z; k++) {
                int val;
                in >> val;
                object.SetVoxelDeferred(dims.z - k - 1, i, j, val);
            }
        }
    }
    // (z, y, x)
    object.ApplyAllDeferredChanges();
}
