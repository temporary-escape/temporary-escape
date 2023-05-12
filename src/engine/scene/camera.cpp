#include "camera.hpp"

using namespace Engine;

Camera::Camera(Matrix4& transform) : transform(&transform) {
}

void Camera::setViewport(const Vector2i& viewport) {
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

void Camera::setProjection(const float fov) {
    fovOrZoom = fov;
    isOrtho = false;
}

void Camera::setOrthographic(const float zoom) {
    fovOrZoom = zoom;
    isOrtho = true;
}

void Camera::setProjectionMatrix(const Matrix4& value) {
    projectionMatrix = value;
}

void Camera::lookAt(const Vector3& eye, const Vector3& target, const Vector3& up) {
    *transform = glm::inverse(glm::lookAt(eye, target, up));
}

Vector3 Camera::screenToWorld(const Vector2& pos) const {
    return Engine::screenToWorld(*transform, projectionMatrix, viewport, pos);
}

Vector2 Camera::worldToScreen(const Vector3& pos, const bool invert) const {
    return Engine::worldToScreen(*transform, projectionMatrix, viewport, pos, invert);
}

std::vector<Vector2> Camera::worldToScreen(const std::vector<Vector3>& pos) const {
    return Engine::worldToScreen(*transform, projectionMatrix, viewport, pos);
}
