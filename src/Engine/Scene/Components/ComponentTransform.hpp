#pragma once

#include "../Camera.hpp"
#include <bullet/LinearMath/btMotionState.h>

class btRigidBody;

namespace Engine {
class ENGINE_API ComponentRigidBody;

enum class TransformFlags : uint64_t {
    Static = 1ULL << 0,
    Kinematic = 1ULL << 1,
};

class ENGINE_API ComponentTransform : public btMotionState, public Component {
public:
    static constexpr auto NullParentId = std::numeric_limits<uint64_t>::max();

    ComponentTransform() = default;
    explicit ComponentTransform(EntityId entity);
    virtual ~ComponentTransform() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentTransform);

    void setParent(const ComponentTransform* value);
    [[nodiscard]] const ComponentTransform* getParent() const {
        return parent;
    }
    [[nodiscard]] uint64_t getParentId() const {
        return parentId;
    }

    void translate(const Vector3& pos);

    void move(const Vector3& pos);

    void rotate(const Vector3& axis, float degrees);
    void rotateX(float degrees);
    void rotateY(float degrees);
    void rotateZ(float degrees);

    void rotate(const Quaternion& q);

    void scale(const Vector3& value);

    [[nodiscard]] Matrix4& getTransform() {
        return transform;
    }

    [[nodiscard]] const Matrix4& getTransform() const {
        return transform;
    }

    [[nodiscard]] const Matrix4& getInterpolatedTransform() const {
        return interpolated ? transformInterpolated : transform;
    }

    [[nodiscard]] Matrix4 getAbsoluteTransform() const;
    [[nodiscard]] Matrix4 getAbsoluteInterpolatedTransform() const;

    void setTransform(const Matrix4& value);

    [[nodiscard]] Vector3 getPosition() const {
        return {transform[3]};
    }

    [[nodiscard]] Vector3 getInterpolatedPosition() const {
        return getInterpolatedTransform()[3];
    }

    Quaternion getOrientation() const;

    [[nodiscard]] Vector3 getAbsolutePosition() const;
    [[nodiscard]] Vector3 getAbsoluteInterpolatedPosition() const;

    float getScaleUniform() const {
        return glm::length(transform[0]);
    }

    void setStatic(bool value);
    bool isStatic() const;

    void setKinematic(bool value);
    bool isKinematic() const;

    void interpolate();

    void setScene(Scene& value);
    void setRigidBody(ComponentRigidBody& value);

    MSGPACK_DEFINE_ARRAY(transform, flags, parentId);

private:
    void getWorldTransform(btTransform& worldTrans) const override;
    void setWorldTransform(const btTransform& worldTrans) override;

private:
    Matrix4 transform{1.0f};
    Matrix4 transformInterpolated{1.0f};
    const ComponentTransform* parent{nullptr};
    uint64_t parentId{NullParentId};
    uint64_t flags{0};
    bool interpolated{false};

    Scene* scene{nullptr};
    ComponentRigidBody* rigidBody{nullptr};
};
} // namespace Engine
