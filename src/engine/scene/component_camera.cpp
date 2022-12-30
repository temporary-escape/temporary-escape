#include "component_camera.hpp"

using namespace Engine;

// TODO
static const Config config{};

void ComponentCamera::update(const float delta) {
    if (!isOrthographic()) {
        auto pos = getObject().getPosition();
        const auto moveFactor = delta * speed * (fast ? 10.0f : 1.0f);

        if (move[0]) {
            pos += Camera::getForward() * moveFactor;
        }
        if (move[1]) {
            pos -= Camera::getForward() * moveFactor;
        }
        if (move[2]) {
            pos -= Camera::getSideways() * moveFactor;
        }
        if (move[3]) {
            pos += Camera::getSideways() * moveFactor;
        }
        if (move[4]) {
            pos += Vector3{0.0f, moveFactor, 0.0f};
        }
        if (move[5]) {
            pos += Vector3{0.0f, -moveFactor, 0.0f};
        }

        getObject().move(pos);
    }
}

/*void ComponentCamera::render(VulkanDevice& vulkan, const Vector2i& viewport) {

    const auto makeUniform = [this, viewport](const bool zero) {
        auto viewMatrix = getViewMatrix();
        if (zero) {
            viewMatrix[3] = Vector4{0.0f, 0.0f, 0.0f, 1.0f};
        }
        const auto transformationProjectionMatrix = getProjectionMatrix() * viewMatrix;
        const auto eyesPos = Vector3(glm::inverse(viewMatrix)[3]);
        const auto projectionViewInverseMatrix = glm::inverse(transformationProjectionMatrix);

        Uniform uniform{};
        uniform.transformationProjectionMatrix = transformationProjectionMatrix;
        uniform.viewProjectionInverseMatrix = projectionViewInverseMatrix;
        uniform.viewMatrix = viewMatrix;
        uniform.projectionMatrix = getProjectionMatrix();
        uniform.viewport = viewport;
        uniform.eyesPos = eyesPos;

        return uniform;
    };

    setViewport(viewport);
    auto uniform = makeUniform(false);

    if (!ubo) {
        ubo = vulkan.createBuffer(VulkanBuffer::Type::Uniform, VulkanBuffer::Usage::Dynamic, sizeof(Uniform));
        ubo.subData(&uniform, 0, sizeof(Uniform));
    } else {
        auto dst = ubo.mapPtr(sizeof(Uniform));
        std::memcpy(dst, &uniform, sizeof(Uniform));
        ubo.unmap();
    }

    uniform = makeUniform(true);

    if (!uboZeroPos) {
        uboZeroPos = vulkan.createBuffer(VulkanBuffer::Type::Uniform, VulkanBuffer::Usage::Dynamic, sizeof(Uniform));
        uboZeroPos.subData(&uniform, 0, sizeof(Uniform));
    } else {
        auto dst = uboZeroPos.mapPtr(sizeof(Uniform));
        std::memcpy(dst, &uniform, sizeof(Uniform));
        uboZeroPos.unmap();
    }
}*/

void ComponentCamera::moveToOrtographic(const Vector3& position) {
    lookAt(position, position - Vector3{0.0f, 1.0f, 0.0f}, Vector3{0.0f, 0.0f, 1.0f});
}

void ComponentCamera::updateRotationFreeLook(const Vector2& diff) {
    static const auto pitchMin = glm::radians(-80.0f);
    static const auto pitchMax = glm::radians(80.0f);
    static const auto deg180 = glm::radians(180.0f);

    const auto mid = Vector2{Camera::getViewport()} / 2.0f;
    const auto target = mid + diff * -0.75f;
    const auto newForward = glm::normalize(Camera::screenToWorld(target));

    auto newPitch = glm::asin(newForward.y);
    const auto newYaw = glm::atan(newForward.x, newForward.z);

    if (newPitch > pitchMax) {
        newPitch = pitchMax;
    }
    if (newPitch < pitchMin) {
        newPitch = pitchMin;
    }

    auto forward = Vector3{0.0f, 0.0f, -1.0f};
    forward = glm::normalize(glm::rotateX(forward, -newPitch));
    forward = glm::normalize(glm::rotateY(forward, newYaw));

    lookAt(getEyesPos(), getEyesPos() + forward);
}

void ComponentCamera::eventMouseMoved(const Vector2i& pos) {
    if (isOrthographic()) {
        if (panFlag) {
            const auto from = screenToWorld(mousePosOld);
            const auto to = screenToWorld(pos);

            const auto diff = Vector3{from.x - to.x, 0.0f, from.z - to.z};
            moveToOrtographic(getObject().getPosition() + diff * Vector3{1.0f, 1.0f, -1.0f});

            mousePosOld = pos;
        }
    } else {
        if (rotationStarted) {
            const auto diff = Vector2{pos} - rotationInputValue;
            rotationInputValue = pos;
            updateRotationFreeLook(diff * Vector2{1.0f, -1.0f});
        }
    }
}

void ComponentCamera::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    if (isOrthographic()) {
        if (button == MouseButton::Right && !panFlag) {
            panFlag = true;
            mousePosOld = pos;
        }
    } else {
        if (button == MouseButton::Right && !rotationStarted) {
            rotationStarted = true;
            rotationInputValue = pos;
        }
    }
}

void ComponentCamera::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    if (isOrthographic()) {
        if (button == MouseButton::Right) {
            panFlag = false;
        }
    } else {
        if (button == MouseButton::Right) {
            rotationStarted = false;
        }
    }
}

void ComponentCamera::eventMouseScroll(const int xscroll, const int yscroll) {
    if (isOrthographic()) {
        auto zoom = getOrthoScale();
        const auto factor = map(zoom, zoomMin, zoomMax, 2.0f, 200.0f);
        zoom += static_cast<float>(-yscroll * 0.1f) * factor;

        if (zoom < zoomMin) {
            zoom = zoomMin;
        } else if (zoom > zoomMax) {
            zoom = zoomMax;
        }

        setOrthographic(zoom);
    }
}

void ComponentCamera::eventKeyPressed(const Key key, const Modifiers modifiers) {
    if (!isOrthographic()) {
        if (config.input.cameraForward(key, modifiers)) {
            move[0] = true;
        }
        if (config.input.cameraBackwards(key, modifiers)) {
            move[1] = true;
        }
        if (config.input.cameraLeft(key, modifiers)) {
            move[2] = true;
        }
        if (config.input.cameraRight(key, modifiers)) {
            move[3] = true;
        }
        if (config.input.cameraUp(key, modifiers)) {
            move[4] = true;
        }
        if (config.input.cameraDown(key, modifiers)) {
            move[5] = true;
        }
        if (config.input.cameraFast(key, modifiers)) {
            fast = true;
        }
    }
}

void ComponentCamera::eventKeyReleased(const Key key, const Modifiers modifiers) {
    if (!isOrthographic()) {
        if (config.input.cameraForward(key, modifiers)) {
            move[0] = false;
        }
        if (config.input.cameraBackwards(key, modifiers)) {
            move[1] = false;
        }
        if (config.input.cameraLeft(key, modifiers)) {
            move[2] = false;
        }
        if (config.input.cameraRight(key, modifiers)) {
            move[3] = false;
        }
        if (config.input.cameraUp(key, modifiers)) {
            move[4] = false;
        }
        if (config.input.cameraDown(key, modifiers)) {
            move[5] = false;
        }
        if (config.input.cameraFast(key, modifiers)) {
            fast = false;
        }
    }
}

void ComponentCamera::eventCharTyped(const uint32_t code) {
}
