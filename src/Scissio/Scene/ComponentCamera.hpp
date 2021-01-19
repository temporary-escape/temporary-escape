#pragma once

#include "../Library.hpp"
#include "../Math/Utils.hpp"
#include "ComponentSystem.hpp"

namespace Scissio {
class Renderer;

class SCISSIO_API ComponentCamera : public Component {
public:
    static constexpr ComponentType Type = 2;

    ComponentCamera() = default;
    ComponentCamera(Object& object) : Component(Type, object) {
    }
    virtual ~ComponentCamera() = default;

    void setProjection(const Vector2i& viewport, const float fov) {
        projectionMatrix =
            glm::perspective(glm::radians(fov), viewport.x / static_cast<float>(viewport.y), 0.1f, 1000.0f);
    }

    Matrix4 getViewMatrix() const {
        return glm::inverse(getObject().getTransform());
    }

    const Matrix4& getProjectionMatrix() const {
        return projectionMatrix;
    }

    bool isActive() const {
        return active;
    }

    Vector3 screenToWorld(const Vector2i& viewport, const Vector2i& pos) const {
        return Scissio::screenToWorld(getObject().getTransform(), glm::inverse(projectionMatrix), viewport, pos);
    }

    Vector3 getEyesPos() const {
        return getObject().getPosition();
    }

private:
    bool active{true};
    Matrix4 projectionMatrix;
};
} // namespace Scissio
