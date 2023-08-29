#include "component_ship_control.hpp"
#include "../../utils/exceptions.hpp"
#include "../entity.hpp"
#include <glm/gtx/euler_angles.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ComponentShipControl::ComponentShipControl(entt::registry& reg, entt::entity handle) : Component{reg, handle} {
}

void ComponentShipControl::update(const float delta, ComponentTransform& transform) {
    float velocityDelta;

    if (velocityTarget > 0.001f && velocityValue < velocityTarget) {
        velocityDelta = delta * 0.2f;
    } else if (velocityTarget < 0.001f && velocityValue <= 0.0f) {
        velocityDelta = delta * 0.1f;
    } else {
        velocityDelta = delta;
    }

    velocityValue = glm::mix(velocityValue, velocityTarget, velocityDelta);

    static const auto pi2 = glm::pi<float>() * 2.0f;
    static const auto maxPitch = glm::radians(45.0f);
    static const auto minPitch = -glm::radians(45.0f);
    static const auto maxRoll = glm::radians(15.0f);
    static const auto minRoll = -glm::radians(15.0f);

    if (directionRelative[0] && !directionRelative[1]) {
        rotation.y += glm::radians(25.0f * delta);
        rotation.z += glm::radians(30.0f * delta);
    } else if (!directionRelative[0] && directionRelative[1]) {
        rotation.y -= glm::radians(25.0f * delta);
        rotation.z -= glm::radians(30.0f * delta);
    } else {
        if (rotation.z > 0.0f) {
            rotation.z -= glm::radians(std::min(rotation.z, 10.0f * delta));
        } else if (rotation.z < 0.0f) {
            rotation.z += glm::radians(std::min(-rotation.z, 10.0f * delta));
        }
    }

    if (directionRelative[2] && !directionRelative[3]) {
        rotation.x -= glm::radians(20.0f * delta);
    } else if (!directionRelative[2] && directionRelative[3]) {
        rotation.x += glm::radians(20.0f * delta);
    } else {
        if (rotation.x > 0.0f) {
            rotation.x -= glm::radians(std::min(rotation.x, 20.0f * delta));
        } else if (rotation.x < 0.0f) {
            rotation.x += glm::radians(std::min(-rotation.x, 20.0f * delta));
        }
    }

    if (rotation.x > maxPitch) {
        rotation.x = maxPitch;
    }
    if (rotation.x < minPitch) {
        rotation.x = minPitch;
    }

    if (rotation.z > maxRoll) {
        rotation.z = maxRoll;
    }
    if (rotation.z < minRoll) {
        rotation.z = minRoll;
    }

    while (rotation.y > pi2) {
        rotation.y -= pi2;
    }
    while (rotation.y < 0.0f) {
        rotation.y += pi2;
    }

    Vector3 forward{0.0f, 0.0f, 1.0f};
    forward = glm::rotateX(forward, rotation.x);
    forward = glm::rotateY(forward, rotation.y);

    auto orientation = glm::quatLookAt(forward, {0.0f, 1.0f, 0.0f});
    // orientation = glm::rotate(orientation, rotation.z, forward);

    const auto modelMatrix = transform.getAbsoluteTransform();
    const auto absolutePosition = Vector3{modelMatrix[3]};

    // Translate forward based on the current speed
    Matrix4 updated{1.0f};
    updated = glm::translate(updated, absolutePosition + -forward * delta * velocityValue);
    updated = updated * glm::toMat4(orientation);

    // Update the transform
    transform.setTransform(updated);
}

void ComponentShipControl::setSpeed(const float value) {
    velocityTarget = value;
}

void ComponentShipControl::setDirection(const Vector3& value) {
    (void)value;
    // TODO
}

void ComponentShipControl::setDirectionRelative(const int leftRight, const int downUp) {
    directionRelative[0] = leftRight == -1;
    directionRelative[1] = leftRight == 1;
    directionRelative[2] = downUp == -1;
    directionRelative[3] = downUp == 1;
}

void ComponentShipControl::patch(entt::registry& reg, entt::entity handle) {
    reg.patch<ComponentShipControl>(handle);
}
