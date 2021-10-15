#include "lit/voxels/voxel_world.hpp"

using namespace lit::voxels;

VoxelWorld::VoxelWorld(glm::ivec3 shadow_dims, float shadow_scale) {
    auto shadow_center = glm::vec3(shadow_dims.x, 0, shadow_dims.z) / 2.f;
    auto transform = transform3(glm::vec3(), glm::quat(1, 0, 0, 0), shadow_scale);
    m_shadow_object = std::make_shared<VoxelObject>(transform, shadow_center, shadow_dims, true);
}

void VoxelWorld::AddObject(std::shared_ptr<VoxelObject> object_to_add) {
    if (!object_to_add) return;
    RasterizeShadow(*object_to_add, object_to_add->GetTransform(), iregion3::all(), false);
    m_objects.emplace_back(std::move(object_to_add));
}

void VoxelWorld::ApplyAllDeferredChanges() {
    std::vector<VoxelObjectChanges> changes;
    for (const auto &object : m_objects) {
        changes.push_back(object->ApplyAllDeferredChanges());
    }

    auto affected_region = [&](size_t i) {
        return (changes[i].transform_changes.num_changes ? iregion3::all()
                                                         : changes[i].grid_changes.affected_region);
    };

    auto any_changes = [&](size_t i) {
        return changes[i].transform_changes.num_changes || changes[i].grid_changes.num_changes;
    };

    for (size_t i = 0; i < m_objects.size(); i++) {
        if (!any_changes(i)) continue;
        RasterizeShadow(*m_objects[i], changes[i].transform_changes.previous_transform, affected_region(i), true);
    }

    for (size_t i = 0; i < m_objects.size(); i++) {
        if (!any_changes(i)) continue;
        RasterizeShadow(*m_objects[i], changes[i].transform_changes.current_transform, affected_region(i), false);
    }
}

double GetIntersection(glm::vec3 pos, glm::vec3 shift, float size) {
    pos = glm::mod(pos, 1.0f);
    auto lo = glm::max(glm::vec3(0) + shift, pos - size * 0.5f);
    auto hi = glm::min(glm::vec3(1) + shift, pos + size * 0.5f);
    return glm::compMul(glm::max(glm::vec3(0), hi - lo)) / (size * size * size);
}

void VoxelWorld::RasterizeShadow(const VoxelObject &object, transform3 transform, iregion3 region, bool clear) {
    auto &shadow = *m_shadow_object; // short alias

    region = region.clamped(iregion3(glm::ivec3(), object.GetDims()));

    iregion3 shadow_region = iregion3::empty();

    for (int i = 0; i < 8; i++) {
        glm::ivec3 corner = (region.end - region.begin) * glm::ivec3(i & 1, (i & 2) >> 1, (i & 4) >> 2) + region.begin;
        glm::vec3 corner_world_pos = transform.apply(glm::vec3(corner) - object.GetCenter());
        glm::vec3 shadow_pos = corner_world_pos / shadow.GetTransform().scale + shadow.GetCenter();

        shadow_region.populate(glm::floor(shadow_pos));
        shadow_region.populate(glm::ceil(shadow_pos));
    }

    shadow_region = shadow_region.clamped(iregion3(glm::ivec3(0), shadow.GetDims()));

    double rel_size = shadow.GetTransform().scale / object.GetTransform().scale;

    int updates = 0;

    for (int x = shadow_region.begin.x; x < shadow_region.end.x; x++) {
        for (int y = shadow_region.begin.y; y < shadow_region.end.y; y++) {
            for (int z = shadow_region.begin.z; z < shadow_region.end.z; z++) {
                glm::vec3 world_pos = (glm::vec3(x, y, z) + 0.5f - shadow.GetCenter()) * shadow.GetTransform().scale;
                glm::vec3 object_pos = transform.apply_inv(world_pos) + object.GetCenter();
                glm::ivec3 object_pos_i = glm::floor(object_pos);
                glm::ivec3 signs = glm::sign(glm::mod(object_pos, 1.0f) - glm::vec3(0.5));

                double intersection = 0.0;

                for(int m = 0; m < 1; m++) {
                    glm::ivec3 axis = glm::ivec3(m & 1, (m & 2) >> 1, (m & 4) >> 2);
                    if (object.GetVoxel(object_pos_i + axis * signs) == 0) continue;
                    intersection += GetIntersection(object_pos, glm::vec3(axis * signs), rel_size);
                }

                if (intersection > 0.5) {
                    shadow.SetVoxelDeferred(x, y, z, !clear);
                    updates++;
                }
            }
        }

        if (updates > 1024*1024) {
            m_shadow_object->ApplyAllDeferredChanges();
            updates = 0;
        }
    }

    m_shadow_object->ApplyAllDeferredChanges();
}

const std::vector<std::shared_ptr<VoxelObject>> &VoxelWorld::GetObjects() const {
    return m_objects;
}

void VoxelWorld::Update() {
    /*if (m_objects.size() > 1) {
        m_objects[1]->TransformDeferred(glm::vec3(), glm::angleAxis(0.02f, glm::vec3(1, 1, 1)), 1.0);
    }
    if (m_objects.size() > 2) {
        m_objects[2]->TransformDeferred(glm::vec3(), glm::angleAxis(0.01f, glm::vec3(0.5, 1, 1)), 1.0);
    }*/
    ApplyAllDeferredChanges();
}
