#include "Camera.hpp"

using namespace Engine;

Camera::Camera(Matrix4& transform) : transform(&transform) {
}

void Camera::setViewport(const Vector2i& value) {
    viewport = value;

    if (isOrtho) {
        zNear = -1000.0f;
        zFar = 1000.0f;

        const auto zoom = fovOrZoom;
        const float ratio = static_cast<float>(viewport.x) / static_cast<float>(viewport.y);
        projectionMatrix = glm::ortho(-(zoom * ratio), zoom * ratio, -(zoom), zoom, zNear, zFar);
    } else {
        zNear = 0.1f;
        zFar = 100000.0f;

        projectionMatrix =
            glm::perspective(glm::radians(fovOrZoom), viewport.x / static_cast<float>(viewport.y), zNear, zFar);
    }
}

Camera::Uniform Camera::createUniform(bool zeroPos) const {
    auto viewMatrixCopy = getViewMatrix();
    if (zeroPos) {
        viewMatrixCopy[3] = Vector4{0.0f, 0.0f, 0.0f, 1.0f};
    }
    const auto transformationProjectionMatrix = getProjectionMatrix() * viewMatrixCopy;
    const auto eyesPos = Vector3(glm::inverse(viewMatrixCopy)[3]);
    const auto projectionViewInverseMatrix = glm::inverse(transformationProjectionMatrix);

    Uniform uniform{};
    uniform.transformationProjectionMatrix = transformationProjectionMatrix;
    uniform.viewProjectionInverseMatrix = projectionViewInverseMatrix;
    uniform.viewMatrix = viewMatrixCopy;
    uniform.projectionMatrix = getProjectionMatrix();
    uniform.viewport = viewport;
    uniform.eyesPos = eyesPos;

    return uniform;
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

void Camera::setViewMatrix(const Matrix4& value) {
    viewMatrix = value;
    *transform = glm::inverse(viewMatrix);
}

void Camera::lookAt(const Vector3& eye, const Vector3& target, const Vector3& up) {
    viewMatrix = glm::lookAt(eye, target, up);
    *transform = glm::inverse(viewMatrix);
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
