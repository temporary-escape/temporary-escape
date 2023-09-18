#pragma once

#include "../library.hpp"
#include "../utils/moveable_copyable.hpp"
#include <memory>

class btCollisionShape;

namespace Engine {
class ENGINE_API CollisionShape {
public:
    CollisionShape();
    ~CollisionShape();
    MOVEABLE(CollisionShape);
    NON_COPYABLE(CollisionShape);

    [[nodiscard]] std::unique_ptr<btCollisionShape> clone() const;

    static inline CollisionShape createSphere(const float radius) {
        return CollisionShape{Type::Sphere, radius};
    }

    static inline CollisionShape createConvexHull(const float* points, size_t count) {
        return CollisionShape{Type::ConvexHull, points, count};
    }

    [[nodiscard]] operator bool() const {
        return type != Type::None;
    }

private:
    enum class Type {
        None,
        Sphere,
        ConvexHull,
    };

    explicit CollisionShape(Type type, float value);
    explicit CollisionShape(Type type, const float* values, size_t count);

    Type type;
    std::unique_ptr<btCollisionShape> shape;
};
} // namespace Engine
