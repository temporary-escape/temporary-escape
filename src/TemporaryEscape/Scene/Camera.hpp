#pragma once

#include "../Library.hpp"
#include "../Math/Utils.hpp"
#include "Component.hpp"

namespace Engine {
class Renderer;

class ENGINE_API Camera {
public:
    Camera() = default;
    explicit Camera(Object& object) : object(&object) {
    }

    void setViewport(const Vector2i& viewport) {
        this->viewport = viewport;

        if (isOrtho) {
            const auto zoom = fovOrZoom;
            const float ratio = static_cast<float>(viewport.x) / static_cast<float>(viewport.y);
            projectionMatrix = glm::ortho(-(zoom * ratio), zoom * ratio, -(zoom), zoom, -1000.0f, 1000.0f);
        } else {
            projectionMatrix =
                glm::perspective(glm::radians(fovOrZoom), viewport.x / static_cast<float>(viewport.y), 0.1f, 2000.0f);
        }
    }

    void setProjection(const float fov) {
        fovOrZoom = fov;
        isOrtho = false;
    }

    void setOrthographic(const float zoom) {
        fovOrZoom = zoom;
        isOrtho = true;
    }

    void lookAt(const Vector3& eye, const Vector3& target, const Vector3& up = Vector3{0.0f, 1.0f, 0.0f}) {
        object->updateTransform(glm::inverse(glm::lookAt(eye, target, up)));
    }

    Matrix4 getViewMatrix() const {
        return glm::inverse(object->getTransform());
    }

    const Matrix4& getProjectionMatrix() const {
        return projectionMatrix;
    }

    Vector3 screenToWorld(const Vector2& pos) const {
        return Engine::screenToWorld(object->getTransform(), projectionMatrix, viewport, pos);
    }

    Vector2 worldToScreen(const Vector3& pos) const {
        return Engine::worldToScreen(object->getTransform(), projectionMatrix, viewport, pos);
    }

    Vector3 getEyesPos() const {
        return object->getPosition();
    }

private:
    Object* object{nullptr};
    Matrix4 projectionMatrix{1.0f};
    Vector2i viewport;
    float fovOrZoom{1.0f};
    bool isOrtho{false};
};
} // namespace Engine
