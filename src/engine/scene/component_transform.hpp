#pragma once

#include "../graphics/shader.hpp"
#include "camera.hpp"
#include "component_user_input.hpp"

namespace Engine {
class ENGINE_API ComponentTransform : public Component {
public:
    ComponentTransform() = default;
    virtual ~ComponentTransform() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentTransform);

    void setParent(const std::shared_ptr<Entity>& entity);
    void removeParent();

    [[nodiscard]] std::shared_ptr<Entity> getParent() const {
        return parent.lock();
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

    void updateTransform(const Matrix4& value);

    [[nodiscard]] Vector3 getPosition() const {
        return {transform[3]};
    }

    [[nodiscard]] Vector3 getAbsolutePosition() const;

private:
    Matrix4 transform{1.0f};
    std::weak_ptr<Entity> parent;
};
} // namespace Engine
