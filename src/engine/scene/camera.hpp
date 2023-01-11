#pragma once

#include "../library.hpp"
#include "../math/utils.hpp"
#include "component.hpp"

namespace Engine {
class Renderer;

class ENGINE_API Camera {
public:
    Camera() = default;
    explicit Camera(Matrix4& transform) : transform(&transform) {
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

    void lookAt(const Vector3& eye, const Vector3& target, const Vector3& up = Vector3{0.0f, -1.0f, 0.0f}) {
        *transform = glm::inverse(glm::lookAt(eye, target, up));
    }

    Vector3 getForward() const {
        return glm::normalize(Vector3{glm::row(getViewMatrix(), 2)});
    }

    Vector3 getSideways() const {
        return glm::normalize(Vector3{glm::row(getViewMatrix(), 0)});
    }

    Matrix4 getViewMatrix() const {
        return glm::inverse(*transform);
    }

    const Matrix4& getProjectionMatrix() const {
        return projectionMatrix;
    }

    Vector3 screenToWorld(const Vector2& pos) const {
        return Engine::screenToWorld(*transform, projectionMatrix, viewport, pos);
    }

    Vector2 worldToScreen(const Vector3& pos, bool invert = false) const {
        return Engine::worldToScreen(*transform, projectionMatrix, viewport, pos, invert);
    }

    std::vector<Vector2> worldToScreen(const std::vector<Vector3>& pos) const {
        return Engine::worldToScreen(*transform, projectionMatrix, viewport, pos);
    }

    Vector3 getEyesPos() const {
        return {(*transform)[3]};
    }

    const Vector2i& getViewport() const {
        return viewport;
    }

    float getPitch() const {
        // return glm::acos(glm::dot(getForward(), Vector3{0.0f, -1.0f, 0.0f}));
        const auto forward = getForward();
        return glm::asin(forward.y);
    }

    float getYaw() const {
        const auto forward = getForward();
        return glm::atan(forward.x, forward.z);
    }

    bool isOrthographic() const {
        return isOrtho;
    }

    float getOrthoScale() const {
        return fovOrZoom;
    }

    Matrix4& getTransform() {
        return *transform;
    }

    const Matrix4& getTransform() const {
        return *transform;
    }

private:
    Matrix4* transform{nullptr};
    Matrix4 projectionMatrix{1.0f};
    Vector2i viewport;
    float fovOrZoom{1.0f};
    bool isOrtho{false};
};
} // namespace Engine
