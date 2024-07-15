#include "ComponentShipControl.hpp"
#include "../../Utils/Exceptions.hpp"
#include "../Scene.hpp"
#include <glm/gtx/euler_angles.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ComponentShipControl::ComponentShipControl(EntityId entity) : Component{entity} {
}

void ComponentShipControl::update(Scene& scene, const float delta, ComponentTransform& ourTransform) {
    constexpr Vector3 upVector{0.0f, 1.0f, 0.0f};
    constexpr auto accelerationAngleMin = glm::radians(30.0f);
    constexpr auto slowdownAngle = glm::radians(15.0f);

    if (replicated || action == ShipAutopilotAction::Idle) {
        return;
    }

    if (approachTarget != NullEntity && recalculateBounds && scene.isValid(approachTarget)) {
        recalculateBounds = false;

        ourBounds = scene.getEntityBounds(getEntity(), ourTransform);
        const auto* theirTransform = scene.tryGetComponent<ComponentTransform>(approachTarget);
        if (theirTransform) {
            targetBounds = scene.getEntityBounds(approachTarget, *theirTransform);
        } else {
            targetBounds = 0.0f;
        }
    }

    // Get our position
    const auto ourPos = ourTransform.getAbsolutePosition();

    // Our current orientation (directly forward)
    const auto ourOrientation = glm::quat_cast(ourTransform.getTransform());
    const auto ourForward = glm::rotate(ourOrientation, Vector3{0.0f, 0.0f, 1.0f});

    std::optional<Vector3> res;
    if (action == ShipAutopilotAction::Approach) {
        res = getSteeringApproach(scene, delta, ourTransform);
    } else if (action == ShipAutopilotAction::KeepDistance) {
        res = getSteeringKeepAtDistance(scene, delta, ourTransform);
    } else if (action == ShipAutopilotAction::Orbit) {
        res = getSteeringOrbit(scene, delta, ourTransform);
    }

    if (!res) {
        actionCancelMovement();
        targetPos = ourPos + -ourForward * (forwardVelocity + 1.0f);
    } else {
        targetPos = *res;
    }

    if (action == ShipAutopilotAction::CancellingRotation && forwardVelocity < 1.0f) {
        targetPos = ourPos + glm::normalize(Vector3{-ourForward.x, 0.0f, -ourForward.z}) * (forwardVelocity + 1.0f);
    }

    // Deceleration is always much faster!
    static const auto decelerationFactor = 5.0f;
    const auto forwardDeceleration = forwardAcceleration * decelerationFactor;

    // Distance to the target position we need to arrive to
    const auto distance = glm::distance(targetPos, ourPos);
    targetDistance = distance;

    // When do we need to slow down?
    const auto secondsToStop = forwardVelocityMax / forwardDeceleration + 1.0f;
    const auto slowDownRadius = 0.5f * forwardDeceleration * glm::pow(secondsToStop, 2.0f);

    float targetSpeed;
    if (distance > slowDownRadius || action == ShipAutopilotAction::Orbit) {
        targetSpeed = forwardVelocityMax;
    } else {
        targetSpeed = forwardVelocityMax * distance / slowDownRadius;
    }

    // logger.warn("targetSpeed: {} secondsToStop: {} slowDownRadius: {}", targetSpeed, secondsToStop, slowDownRadius);

    if (targetSpeed < 0.5f) {
        targetSpeed = 0.0f;
    }

    // Did we arrive to the destination?
    if (action == ShipAutopilotAction::Approach && distance < 10.0f) {
        actionCancelMovement();
    }

    // Do not move forward if we are entering an idle state
    if (action == ShipAutopilotAction::CancellingRotation) {
        targetSpeed = 0.0f;
    }

    // Figure out which direction we should be facing
    const auto lookAtPos = ourPos - (targetPos - ourPos);
    Matrix4 alignedTransform;
    if (action == ShipAutopilotAction::Orbit /* && glm::distance(ourPos, orbitOrigin) < orbitDistance + 1000.0f*/) {
        alignedTransform = glm::inverse(glm::lookAt(ourPos, lookAtPos, orbitNormal));
    } else {
        alignedTransform = glm::inverse(glm::lookAt(ourPos, lookAtPos, {0.0f, 1.0f, 0.0f}));
    }
    auto targetOrientation = glm::quat_cast(alignedTransform);

    // Calculate the angle between out forward and the target direction we should be facing
    auto targetForward = glm::rotate(targetOrientation, Vector3{0.0f, 0.0f, 1.0f});

    auto angle = glm::acos(glm::dot(targetForward, ourForward));
    if (std::isnan(angle) || angle < 0.001f) {
        angle = 0.0f;
        if (action == ShipAutopilotAction::CancellingRotation && forwardVelocity < 1.0f) {
            action = ShipAutopilotAction::Idle;
        }
    }

    // Calculate the new orientation, delta time adjusted, to rotate towards the target
    Quaternion newOrientation;
    if (distance > 10.0f || action == ShipAutopilotAction::CancellingRotation) {
        const auto currentAngularVelocity = /*aligning ? angularVelocity * 0.2f : */ angularVelocity;
        const auto minAVel = currentAngularVelocity * 0.2f;
        const auto aVel = angle < slowdownAngle ? map(angle, 0.0f, slowdownAngle, minAVel, currentAngularVelocity)
                                                : currentAngularVelocity;

        auto factor = glm::clamp((aVel * delta) / angle, 0.0f, 1.0f);
        if (action == ShipAutopilotAction::CancellingRotation) {
            factor *= 0.3f;
        }
        newOrientation = glm::slerp(ourOrientation, targetOrientation, factor);
    } else {
        newOrientation = ourOrientation;
    }

    // Do not accelerate, if the angle towards the target is too big
    if (angle > accelerationAngleMin) {
        targetSpeed = 0.0f;
    }

    // Update our current acceleration
    float velocityDiff;
    if (forwardVelocity < targetSpeed) {
        velocityDiff = glm::clamp(targetSpeed - forwardVelocity, 0.0f, forwardAcceleration) * delta;
    } else {
        velocityDiff = -glm::clamp(forwardVelocity - targetSpeed, 0.0f, forwardDeceleration) * delta;
        // logger.warn("Slowing down by: {}", velocityDiff);
    }
    forwardVelocity += velocityDiff;

    // The new ship transform
    auto shipTransform = glm::translate(Matrix4{1.0f}, ourPos);
    shipTransform = shipTransform * glm::toMat4(newOrientation);

    // Apply the velocity
    shipTransform = glm::translate(shipTransform, Vector3{0.0f, 0.0f, -1.0f} * forwardVelocity * delta);

    // Update the ship transform
    ourTransform.setTransform(shipTransform);
    scene.setDirty(ourTransform);
    scene.setDirty(*this);
}

void ComponentShipControl::addTurret(ComponentTurret& turret) {
    turrets.push_back(&turret);
}

std::optional<Vector3> ComponentShipControl::getSteeringApproach(Scene& scene, const float delta,
                                                                 ComponentTransform& ourTransform) {
    (void)delta;

    // Cancel approach action if invalid entity
    if (approachTarget == entity || !scene.valid(approachTarget)) {
        return std::nullopt;
    }

    // Cancel approach if the entity has no transform
    const auto* theirTransform = scene.tryGetComponent<ComponentTransform>(approachTarget);
    if (!theirTransform) {
        return std::nullopt;
    }

    // Their position
    auto theirPos = theirTransform->getAbsolutePosition();

    // Get vector towards the target
    auto direction = theirPos - ourTransform.getAbsolutePosition();
    direction = glm::normalize(direction);

    // Subtract entity size from the approach pos
    theirPos -= direction * targetBounds;

    // The target pos is the position we need to arrive to
    return theirPos;
}

std::optional<Vector3> ComponentShipControl::getSteeringKeepAtDistance(Scene& scene, float delta,
                                                                       ComponentTransform& ourTransform) {
    const auto res = getSteeringApproach(scene, delta, ourTransform);
    if (!res) {
        return std::nullopt;
    }

    // Get vector towards the target
    const auto ourPos = ourTransform.getAbsolutePosition();
    auto direction = *res - ourPos;
    direction = glm::normalize(direction);

    // Use the direction towards the target as the line at which we should keep the distance at
    return *res - direction * keepAtDistance;
}

std::optional<Vector3> ComponentShipControl::getSteeringOrbit(Scene& scene, const float delta,
                                                              ComponentTransform& ourTransform) {
    (void)delta;

    // Cancel approach action if invalid entity
    if (approachTarget == entity || !scene.valid(approachTarget)) {
        return std::nullopt;
    }

    // Cancel approach if the entity has no transform
    const auto* theirTransform = scene.tryGetComponent<ComponentTransform>(approachTarget);
    if (!theirTransform) {
        return std::nullopt;
    }

    // Their position
    auto theirPos = theirTransform->getAbsolutePosition();

    // Our position
    const auto ourPos = ourTransform.getAbsolutePosition();

    if (!orbitMatrixChosen) {
        orbitMatrixChosen = true;
        orbitMatrix = glm::inverse(glm::lookAt(ourPos, theirPos, {0.0f, 1.0f, 0.0f}));
        orbitMatrix = glm::rotate(orbitMatrix, glm::radians(180.0f), Vector3{0.0f, 1.0f, 0.0f});
        orbitMatrix[3] = Vector4{0.0f, 0.0f, 0.0f, 1.0f};

        // Get the orbit circle normal vector (up)
        orbitNormal = glm::normalize(Vector3{orbitMatrix * Vector4{0.0f, 1.0f, 0.0f, 0.0f}});
    }

    orbitOrigin = theirPos;

    // How far should we orbit?
    orbitDistance = keepAtDistance + targetBounds + ourBounds;

    // Get the direction to our ship
    const auto orbitToShip = ourPos - theirPos;

    // Distance from our position to the orbit plane
    const auto distPlane = glm::dot(orbitToShip, orbitNormal);

    // Our projected position on the orbit plane
    const auto orbitPlaneProjected = ourPos - distPlane * orbitNormal;

    // Distance to the orbit center
    const auto dist = glm::distance(orbitPlaneProjected, theirPos);

    // Calculate distance to the orbit tangent
    const auto tangentDistance = glm::sqrt(glm::pow(dist, 2.0f) - glm::pow(orbitDistance, 2.0f));

    // Vector towards the orbit center
    const auto toOrbitCenterMat = glm::inverse(glm::lookAt(orbitPlaneProjected, theirPos, {0.0f, 1.0f, 0.0f}));
    const auto toTangentDir = Vector3{toOrbitCenterMat * Vector4{0.0f, 0.0f, 1.0f, 0.0f}} * tangentDistance;
    const auto tangentAngle = glm::acos(tangentDistance / dist);
    const auto tangentPos = orbitPlaneProjected + glm::rotate(toTangentDir, -tangentAngle, orbitNormal);

    const auto distToTangentPos = glm::distance(ourPos, tangentPos);

    if (distToTangentPos > 10.0f) {
        return tangentPos;
    }

    const auto centerToPoint = glm::normalize(orbitPlaneProjected - theirPos) * orbitDistance;

    const auto rotated = glm::rotate(centerToPoint, glm::radians(5.0f), orbitNormal);

    return theirPos + rotated;
}

void ComponentShipControl::actionApproach(const EntityId target) {
    if (target != approachTarget) {
        recalculateBounds = true;
    }
    action = ShipAutopilotAction::Approach;
    approachTarget = target;
}

void ComponentShipControl::actionKeepDistance(const EntityId target, const float distance) {
    if (target != approachTarget) {
        recalculateBounds = true;
    }
    action = ShipAutopilotAction::KeepDistance;
    approachTarget = target;
    keepAtDistance = distance;
}

void ComponentShipControl::actionCancelMovement() {
    action = ShipAutopilotAction::CancellingRotation;
    approachTarget = NullEntity;
    targetDistance = 0.0f;
}

void ComponentShipControl::actionOrbit(const EntityId target, const float radius) {
    if (target != approachTarget) {
        recalculateBounds = true;
    }
    action = ShipAutopilotAction::Orbit;
    approachTarget = target;
    keepAtDistance = radius;
    orbitMatrixChosen = false;
}

void ComponentShipControl::setReplicated(const bool value) {
    replicated = value;
}
