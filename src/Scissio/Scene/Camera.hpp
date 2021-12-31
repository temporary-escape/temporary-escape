#pragma once

#include "../Library.hpp"
#include "../Math/Utils.hpp"
#include "Component.hpp"

namespace Scissio {
class Renderer;

class SCISSIO_API Camera {
public:
    Camera() = default;
    explicit Camera(Object& object) : object(&object) {
    }

    void setProjection(const Vector2i& viewport, const float fov) {
        projectionMatrix =
            glm::perspective(glm::radians(fov), viewport.x / static_cast<float>(viewport.y), 0.1f, 2000.0f);
    }

    void lookAt(const Vector3& eye, const Vector3& target) {
        object->updateTransform(glm::inverse(glm::lookAt(eye, target, Vector3{0.0f, 1.0f, 0.0f})));
    }

    Matrix4 getViewMatrix() const {
        return glm::inverse(object->getTransform());
    }

    const Matrix4& getProjectionMatrix() const {
        return projectionMatrix;
    }

    Vector3 screenToWorld(const Vector2i& viewport, const Vector2i& pos) const {
        return Scissio::screenToWorld(object->getTransform(), projectionMatrix, viewport, pos);
    }

    Vector2i worldToScreen(const Vector2i& viewport, const Vector3& pos) const {
        return Scissio::worldToScreen(object->getTransform(), projectionMatrix, viewport, pos);
    }

    Vector3 getEyesPos() const {
        return object->getPosition();
    }

private:
    Object* object{nullptr};
    Matrix4 projectionMatrix{1.0f};
};
} // namespace Scissio
