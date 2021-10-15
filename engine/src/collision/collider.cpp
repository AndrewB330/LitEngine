#include <engine/collision/collider.hpp>

Collider::Collider(std::unique_ptr<ConvexShape> shape) : shape(std::move(shape)) {
    Collider::TransformUpdated();
}

Vec3 Collider::GetSupportingVector(const Vec3 &dir) const {
    auto rotation_inv = tquat(-rotation.w, rotation.x, rotation.y, rotation.z);
    return translation + glm::rotate(rotation, shape->GetSupportingVector(glm::rotate(rotation_inv, dir)));
}

void Collider::TransformUpdated() const {
    bbox.min.x = GetSupportingVector(Vec3(-1, 0, 0)).x;
    bbox.max.x = GetSupportingVector(Vec3(1, 0, 0)).x;

    bbox.min.y = GetSupportingVector(Vec3(0, -1, 0)).y;
    bbox.max.y = GetSupportingVector(Vec3(0, 1, 0)).y;

    bbox.min.z = GetSupportingVector(Vec3(0, 0, -1)).z;
    bbox.max.z = GetSupportingVector(Vec3(0, 0, 1)).z;
    m_oldTransform = Transform(translation, rotation, scale);
}

const BoundingBox &Collider::GetBBox() const {
    if (!(Transform(translation, rotation, scale) == m_oldTransform)) {
        TransformUpdated();
    }
    return bbox;
}

const ConvexShape *Collider::GetOrigShape() const {
    return shape.get();
}

std::unique_ptr<Collider> CreateBoxCollider(double size, Vec3 pos, Quat rot) {
    auto shape = std::make_unique<BoxShape>(size);
    auto collider = std::make_unique<Collider>(std::move(shape));
    collider->translation = pos;
    collider->rotation = rot;
    return collider;
}

std::unique_ptr<Collider> CreateBoxCollider(Vec3 size, Vec3 pos, Quat rot) {
    auto shape = std::make_unique<BoxShape>(size.x, size.y, size.z);
    auto collider = std::make_unique<Collider>(std::move(shape));
    collider->translation = pos;
    collider->rotation = rot;
    return collider;
}

std::unique_ptr<Collider> CreateSphereCollider(double r, Vec3 pos, Quat rot) {
    auto shape = std::make_unique<SphereShape>(r);
    auto collider = std::make_unique<Collider>(std::move(shape));
    collider->translation = pos;
    collider->rotation = rot;
    return collider;
}