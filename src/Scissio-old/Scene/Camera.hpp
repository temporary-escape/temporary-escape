#pragma once

#include "../Library.hpp"
#include "../Math/Utils.hpp"
#include "Component.hpp"

namespace Scissio {
class Renderer;

class SCISSIO_API Camera : public Object {
public:
    void setProjection(const Vector2i& viewport, const float fov) {
        projectionMatrix =
            glm::perspective(glm::radians(fov), viewport.x / static_cast<float>(viewport.y), 0.1f, 1000.0f);
    }

    void lookAt(const Vector3& eye, const Vector3& target) {
        this->updateTransform(glm::inverse(glm::lookAt(eye, target, Vector3{0.0f, 1.0f, 0.0f})));
    }

    Matrix4 getViewMatrix() const {
        return glm::inverse(getTransform());
    }

    const Matrix4& getProjectionMatrix() const {
        return projectionMatrix;
    }

    Vector3 screenToWorld(const Vector2i& viewport, const Vector2i& pos) const {
        return Scissio::screenToWorld(getTransform(), projectionMatrix, viewport, pos);
    }

    Vector2i worldToScreen(const Vector2i& viewport, const Vector3& pos) const {
        return Scissio::worldToScreen(getTransform(), projectionMatrix, viewport, pos);
    }

    Vector3 getEyesPos() const {
        return getPosition();
    }

private:
    Matrix4 projectionMatrix{1.0f};
};
} // namespace Scissio
