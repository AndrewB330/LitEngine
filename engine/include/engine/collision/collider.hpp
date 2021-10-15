#pragma once

#include <memory>
#include "bounding_volume.hpp"
#include "shape.hpp"
#include "../math.hpp"

class Collider : public ConvexShape, public Transform {
public:
    ~Collider() override = default;

    Collider(std::unique_ptr<ConvexShape> shape);

    Vec3 GetSupportingVector(const Vec3 &dir) const override;

    const BoundingBox &GetBBox() const;

    // TODO: DEPRECATED!
    const ConvexShape* GetOrigShape() const;

private:

    void TransformUpdated() const;

    std::unique_ptr<ConvexShape> shape;

    mutable BoundingBox bbox;
    mutable Transform m_oldTransform;
};

std::unique_ptr<Collider> CreateBoxCollider(double size, Vec3 pos = Vec3(), Quat rot = Quat(1, 0, 0, 0));

std::unique_ptr<Collider> CreateBoxCollider(Vec3 size, Vec3 pos = Vec3(), Quat rot = Quat(1, 0, 0, 0));

std::unique_ptr<Collider> CreateSphereCollider(double r, Vec3 pos = Vec3(), Quat rot = Quat(1, 0, 0, 0));
