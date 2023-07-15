#pragma once

#include "../camera.hpp"

namespace Engine {
enum class TransformFlags : uint64_t {
    Static = 1ULL << 0,
};

class ENGINE_API ComponentTransform : public Component {
public:
    ComponentTransform() = default;
    explicit ComponentTransform(entt::registry& reg, entt::entity handle);
    virtual ~ComponentTransform() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentTransform);

public:
    void setParent(const ComponentTransform& value);
    void removeParent();

    [[nodiscard]] const ComponentTransform* getParent() const {
        return parent;
    }

    void translate(const Vector3& pos);

    void move(const Vector3& pos);

    void rotate(const Vector3& axis, float degrees);

    void rotate(const Quaternion& q);

    void scale(const Vector3& value);

    [[nodiscard]] Matrix4& getTransform() {
        return transform;
    }

    [[nodiscard]] const Matrix4& getTransform() const {
        return transform;
    }

    [[nodiscard]] Matrix4 getAbsoluteTransform() const;

    void setTransform(const Matrix4& value);

    [[nodiscard]] Vector3 getPosition() const {
        return {transform[3]};
    }

    [[nodiscard]] Vector3 getAbsolutePosition() const;

    static void bind(Lua& lua);

    void setStatic(const bool value);
    bool isStatic() const;

    MSGPACK_DEFINE_ARRAY(transform, flags);

protected:
    void patch(entt::registry& reg, entt::entity handle) override;

private:
    Matrix4 transform{1.0f};
    const ComponentTransform* parent{nullptr};
    uint64_t flags{0};
};
} // namespace Engine
