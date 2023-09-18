#include "collision_shape.hpp"
#include "../utils/exceptions.hpp"
#include <btBulletDynamicsCommon.h>

using namespace Engine;

CollisionShape::CollisionShape() = default;

CollisionShape::~CollisionShape() = default;

std::unique_ptr<btCollisionShape> CollisionShape::clone() const {
    switch (type) {
    case Type::Sphere: {
        const auto* sphere = static_cast<btSphereShape*>(shape.get());
        return std::make_unique<btSphereShape>(sphere->getRadius());
    }
    case Type::ConvexHull: {
        const auto* convexHull = static_cast<btConvexHullShape*>(shape.get());
        return std::make_unique<btConvexHullShape>(&convexHull->getPoints()->x(), convexHull->getNumPoints());
    }
    default: {
        EXCEPTION("Empty collision shape");
    }
    }
}

CollisionShape::CollisionShape(CollisionShape::Type type, float value) : type{type} {
    shape = std::make_unique<btSphereShape>(value);
}

CollisionShape::CollisionShape(CollisionShape::Type type, const float* values, size_t count) : type{type} {
    shape = std::make_unique<btConvexHullShape>(values, count, sizeof(float) * 3);
}
