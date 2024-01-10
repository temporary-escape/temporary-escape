#include "ComponentShipControl.hpp"
#include "../../Utils/Exceptions.hpp"
#include "../Entity.hpp"
#include <glm/gtx/euler_angles.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ComponentShipControl::ComponentShipControl(entt::registry& reg, entt::entity handle) : Component{reg, handle} {
}

void ComponentShipControl::update(EntityRegistry& reg, const float delta, ComponentTransform& transform) {
    constexpr auto accelerationAngleMin = glm::radians(30.0f);
    constexpr auto slowdownAngle = glm::radians(15.0f);

    if (!active) {
        return;
    }

    // Target information
    const auto& targetPos = getTargetPos(reg);
    const auto targetDistance = glm::distance(transform.getPosition(), targetPos);

    // The orientation towards the target
    auto newTransform = glm::inverse(glm::lookAt(transform.getPosition(), targetPos, {0.0f, 1.0f, 0.0f}));
    newTransform = glm::rotate(newTransform, glm::radians(180.0f), Vector3{0.0f, 1.0f, 0.0f});
    auto targetOrientation = glm::quat_cast(newTransform);

    // Our current orientation (directly forward)
    const auto ourOrientation = glm::quat_cast(transform.getTransform());

    // Calculate the angle between out forward and the target direction we should be facing
    auto targetForward = glm::rotate(targetOrientation, Vector3{0.0f, 0.0f, 1.0f});
    const auto ourForward = glm::rotate(ourOrientation, Vector3{0.0f, 0.0f, 1.0f});
    auto angle = glm::acos(glm::dot(targetForward, ourForward));
    if (std::isnan(angle)) {
        angle = 0.0f;
    }

    // Calculate the velocity we should apply in order to approach the target or stop
    float velocityDiff;
    auto slowing{false};
    {
        const auto secondsToStop = forwardVelocity / forwardAcceleration;
        const auto distanceToStop = (1.0f / 2.0f) * forwardAcceleration * std::pow(secondsToStop, 2.0f);

        // logger.info("secondsToStop: {} distanceToStop: {}", secondsToStop, distanceToStop);

        if (targetDistance > distanceToStop + 100.0f && angle < accelerationAngleMin) {
            forwardVelocityTarget = forwardVelocityMax;
        } else {
            forwardVelocityTarget = 0.0f;
            slowing = true;
            // forwardVelocity = 0.0f;
        }

        velocityDiff = forwardVelocityTarget - forwardVelocity;
        if (glm::abs(velocityDiff) > forwardAcceleration) {
            if (velocityDiff > 0.0f) {
                velocityDiff = forwardAcceleration;
            } else {
                velocityDiff = forwardAcceleration * -2.0f;
            }
        }
    }

    // Align the ship horizontally if the ship is stopped
    auto aligning{false};
    if (std::abs(velocityDiff) < 0.1f && targetDistance < 100.0f) {
        // Get vector towards the front of the ship, but horizontally aligned
        const auto forwardAligned = glm::normalize(Vector3{ourForward.x, 0.0f, ourForward.z});

        // Use that forward vector to get a new orientation
        auto alignedTransform = glm::inverse(
            glm::lookAt(transform.getPosition(), transform.getPosition() + forwardAligned, {0.0f, 1.0f, 0.0f}));
        targetOrientation = glm::quat_cast(alignedTransform);

        angle = glm::acos(glm::dot(forwardAligned, ourForward));
        if (std::isnan(angle)) {
            angle = 0.0f;
        }

        aligning = true;
    }

    // Calculate the new orientation, delta time adjusted, to rotate towards the target
    Quaternion newOrientation;
    {
        const auto currentAngularVelocity = aligning ? angularVelocity * 0.2f : angularVelocity;
        const auto minAVel = currentAngularVelocity * 0.2f;
        const auto aVel = angle < slowdownAngle ? map(angle, 0.0f, slowdownAngle, minAVel, currentAngularVelocity)
                                                : currentAngularVelocity;

        const auto factor = glm::clamp((aVel * delta) / angle, 0.0f, 1.0f);
        newOrientation = glm::slerp(ourOrientation, targetOrientation, factor);
    }

    // logger.info("velocityDiff: {}", velocityDiff);

    forwardVelocity += velocityDiff * delta;

    // The new ship transform
    auto shipTransform = glm::translate(Matrix4{1.0f}, transform.getPosition());

    // Apply the orientation towards the target
    shipTransform = shipTransform * glm::toMat4(newOrientation);

    // Apply the velocity
    shipTransform = glm::translate(shipTransform, Vector3{0.0f, 0.0f, -1.0f} * forwardVelocity * delta);

    // Update the ship transform
    transform.setTransform(shipTransform);
    reg.patch<ComponentTransform>(getEntity());
}

Vector3 ComponentShipControl::getTargetPos(EntityRegistry& reg) {
    // Is the target entity valid?
    if (!reg.valid(approachTarget)) {
        approachTarget = NullEntity;
        setActive(false);
        return {0.0f, 0.0f, 0.0f};
    }

    // Does the entity have transform component?
    const auto* targetTransform = reg.try_get<ComponentTransform>(approachTarget);
    if (!targetTransform) {
        approachTarget = NullEntity;
        setActive(false);
        return {0.0f, 0.0f, 0.0f};
    }

    return targetTransform->getPosition();
}

void ComponentShipControl::actionApproach(EntityId target) {
    approachTarget = target;
    setActive(true);
}

/*void ComponentShipControl::setSpeed(const float value) {
    velocityTarget = value;
    if (velocityTarget > velocityMax) {
        velocityTarget = velocityMax;
    }
}*/

/*void ComponentShipControl::setSpeedMax(const float value) {
    velocityMax = value;
}*/

/*void ComponentShipControl::setDirection(const Vector3& value) {
    (void)value;
    // TODO
}*/

void ComponentShipControl::setActive(const bool value) {
    active = value;
}

/*void ComponentShipControl::setSpeedBoost(bool value) {
    velocityBoost = value;
}*/

void ComponentShipControl::addTurret(ComponentTurret& turret) {
    turrets.push_back(&turret);
}

/*void ComponentShipControl::setDirectionRelative(const int leftRight, const int downUp) {
    directionRelative[0] = leftRight == -1;
    directionRelative[1] = leftRight == 1;
    directionRelative[2] = downUp == -1;
    directionRelative[3] = downUp == 1;
}*/

void ComponentShipControl::patch(entt::registry& reg, entt::entity handle) {
    reg.patch<ComponentShipControl>(handle);
}
