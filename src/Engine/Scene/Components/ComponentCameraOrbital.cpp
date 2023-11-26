#include "ComponentCameraOrbital.hpp"
#include "ComponentTransform.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ComponentCameraOrbital::ComponentCameraOrbital(entt::registry& reg, entt::entity handle, ComponentCamera& camera) :
    Component{reg, handle}, camera{&camera} {
}

void ComponentCameraOrbital::update(const float delta) {
    distanceValue = lerp(distanceValue, distanceTarget, 8.0f * delta);

    Vector3 offset{distanceValue, 0.0f, 0.0f};
    offset = glm::rotateZ(offset, glm::radians(rotation.x));
    offset = glm::rotateY(offset, glm::radians(rotation.y));
    const auto eyes = target - offset;
    camera->lookAt(eyes, target);
    camera->setPanning(panning);
}

void ComponentCameraOrbital::setTarget(const Vector3& value) {
    target = value;
}

void ComponentCameraOrbital::setDistance(const float value) {
    distanceValue = value;
    distanceTarget = value;
}

void ComponentCameraOrbital::setRotation(const Vector2& value) {
    rotation = value;
}

void ComponentCameraOrbital::setDistanceRange(const float min, const float max) {
    distanceRange = {min, max};
}

void ComponentCameraOrbital::eventMouseMoved(const Vector2i& pos) {
    if (rotationStarted && !panning) {
        panning = true;
    }

    if (panning) {
        auto diff = Vector2{pos} - mousePosOld;
        mousePosOld = pos;
        diff = {diff.y * -0.1f, diff.x * -0.1f};

        rotation += diff;

        if (rotation.x > 80.0f) {
            rotation.x = 80.0f;
        }
        if (rotation.x < -80.0f) {
            rotation.x = -80.0f;
        }

        while (rotation.y > 360.0f) {
            rotation.y -= 360.0f;
        }
        while (rotation.y < 0.0f) {
            rotation.y += 360.0f;
        }
    }
}

void ComponentCameraOrbital::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    if (button == MouseButton::Right && !rotationStarted) {
        rotationStarted = true;
        mousePosOld = pos;
    }
}

void ComponentCameraOrbital::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    if (button == MouseButton::Right && rotationStarted) {
        rotationStarted = false;
        panning = false;
    }
}

void ComponentCameraOrbital::eventMouseScroll(const int xscroll, const int yscroll) {
    const auto factor = map(distanceTarget, 1.0f, distanceRange.y, 1.0f, 250.0f);
    distanceTarget += static_cast<float>(yscroll) * factor;

    if (distanceTarget < distanceRange.x) {
        distanceTarget = distanceRange.x;
    }
    if (distanceTarget > distanceRange.y) {
        distanceTarget = distanceRange.y;
    }
}

void ComponentCameraOrbital::eventKeyPressed(const Key key, const Modifiers modifiers) {
}

void ComponentCameraOrbital::eventKeyReleased(const Key key, const Modifiers modifiers) {
}

void ComponentCameraOrbital::eventCharTyped(uint32_t code) {
}
