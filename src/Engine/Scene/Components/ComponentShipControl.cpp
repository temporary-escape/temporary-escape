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

    Vector3 targetPos;
    if (!res) {
        actionCancelMovement();
        targetPos = ourPos + ourForward;
    } else {
        targetPos = *res;
    }

    // Distance to the target position we need to arrive to
    const auto distance = glm::distance(targetPos, ourPos);
    targetDistance = distance;

    // When do we need to slow down?
    const auto secondsToStop = forwardVelocityMax / forwardAcceleration;
    const auto slowDownRadius = 0.5f * forwardAcceleration * glm::pow(secondsToStop, 2.0f);

    float targetSpeed;
    if (distance > slowDownRadius) {
        targetSpeed = forwardVelocityMax;
    } else {
        targetSpeed = forwardVelocityMax * distance / slowDownRadius;
    }

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
    const auto alignedTransform = glm::inverse(glm::lookAt(ourPos, lookAtPos, {0.0f, 1.0f, 0.0f}));
    const auto targetOrientation = glm::quat_cast(alignedTransform);

    // Get vector towards the front of the ship, but horizontally aligned
    // const auto forwardAligned = glm::normalize(Vector3{ourForward.x, 0.0f, ourForward.z});

    // Calculate the angle between out forward and the target direction we should be facing
    auto targetForward = glm::rotate(targetOrientation, Vector3{0.0f, 0.0f, 1.0f});
    auto angle = glm::acos(glm::dot(targetForward, ourForward));
    if (std::isnan(angle)) {
        angle = 0.0f;
    }

    // Calculate the new orientation, delta time adjusted, to rotate towards the target
    Quaternion newOrientation;
    if (distance > 10.0f) {
        const auto currentAngularVelocity = /*aligning ? angularVelocity * 0.2f : */ angularVelocity;
        const auto minAVel = currentAngularVelocity * 0.2f;
        const auto aVel = angle < slowdownAngle ? map(angle, 0.0f, slowdownAngle, minAVel, currentAngularVelocity)
                                                : currentAngularVelocity;

        const auto factor = glm::clamp((aVel * delta) / angle, 0.0f, 1.0f);
        newOrientation = glm::slerp(ourOrientation, targetOrientation, factor);
    } else {
        newOrientation = ourOrientation;
    }

    // Do not accelerate, if the angle towards the target is too big
    if (angle > accelerationAngleMin) {
        targetSpeed = 0.0f;

        // Do not rotate if the current velocity is too fast
        /*if (forwardVelocity > forwardVelocityMax * 0.25f) {
            newOrientation = ourOrientation;
        }*/
    }

    // Update our current acceleration
    float velocityDiff;
    if (forwardVelocity < targetSpeed) {
        velocityDiff = glm::clamp(targetSpeed - forwardVelocity, 0.0f, forwardAcceleration) * delta;
    } else {
        velocityDiff = -glm::clamp(forwardVelocity - targetSpeed, 0.0f, forwardAcceleration) * delta;
    }
    forwardVelocity += velocityDiff;

    /*logger.debug("targetPos: {} secondsToStop: {} slowDownRadius: {} forwardVelocity: {}",
                 targetPos,
                 secondsToStop,
                 slowDownRadius,
                 forwardVelocity);*/

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
    auto targetPos = theirTransform->getAbsolutePosition();

    // Get vector towards the target
    auto direction = targetPos - ourTransform.getAbsolutePosition();
    direction = glm::normalize(direction);

    // Subtract entity size from the approach pos
    const auto bounds = scene.getEntityBounds(approachTarget, *theirTransform);
    targetPos -= direction * bounds;

    // The target pos is the position we need to arrive to
    return targetPos;
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
}

void ComponentShipControl::actionApproach(const EntityId target) {
    action = ShipAutopilotAction::Approach;
    approachTarget = target;
}

void ComponentShipControl::actionKeepDistance(const EntityId target, const float distance) {
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
    action = ShipAutopilotAction::Orbit;
    approachTarget = target;
    keepAtDistance = radius;
}

void ComponentShipControl::setReplicated(const bool value) {
    replicated = value;
}

/*void ComponentShipControl::update(Scene& scene, const float delta, ComponentTransform& transform) {
    constexpr Vector3 upVector{0.0f, 1.0f, 0.0f};
    constexpr auto accelerationAngleMin = glm::radians(30.0f);
    constexpr auto slowdownAngle = glm::radians(15.0f);

    if (!active) {
        return;
    }

    // Do we have an arbitrary approach direction?
    const auto hasApproachDir = glm::length2(approachDir) > 0;
    Vector3 targetPos = approachPos;
    float computedDistance{0.0f};

    // Is the target entity valid?
    if (approachTarget != entity && scene.valid(approachTarget)) {
        // Does the entity have transform component?
        const auto* targetTransform = scene.tryGetComponent<ComponentTransform>(approachTarget);
        if (!targetTransform) {
            // Invalid entity, stop the ship
            actionStopMovement();
        } else {
            // Remember the approach position
            approachPos = targetTransform->getPosition();

            // Target information
            targetPos = approachPos;
            approachDistance = glm::distance(transform.getPosition(), approachPos);

            // Subtract target entity bounds (if it is an entity)
            approachDistance -= scene.getEntityBounds(approachTarget, *targetTransform);

            // Subtract our own bounds
            approachDistance -= scene.getEntityBounds(entity, transform);

            if (approachDistance < 0.0f) {
                approachDistance = 0.0f;
            }
            computedDistance = approachDistance;
        }
    } else if (hasApproachDir) {
        computedDistance = 10000.0f;
        targetPos = transform.getPosition() + approachDir * computedDistance;
    } else {
        // Invalid entity or no target, stop the ship
        actionStopMovement();
    }

    // Should we do an orbit?
    if (orbitRadius > 0.1f) {
        if (!orbitMatrixChosen) {
            orbitMatrixChosen = true;
            orbitMatrix = glm::inverse(glm::lookAt(transform.getPosition(), targetPos, {0.0f, 1.0f, 0.0f}));
            orbitMatrix = glm::rotate(orbitMatrix, glm::radians(180.0f), Vector3{0.0f, 1.0f, 0.0f});
            orbitMatrix[3] = Vector4{0.0f, 0.0f, 0.0f, 1.0f};
        }

        orbitOrigin = targetPos;

        // Get the orbit circle normal vector (up)
        const auto orbitNormal = Vector3{orbitMatrix * Vector4{0.0f, 1.0f, 0.0f, 0.0f}};

        // Get the direction to our ship
        const auto orbitToShip = transform.getPosition() - targetPos;

        // Get the distance of us towards the orbit plane
        const auto dist = glm::dot(orbitToShip, orbitNormal);

        // Project our position onto the orbit plane
        auto orbitPlaneProjected = transform.getPosition() - dist * orbitNormal;

        // More the projected point to match the orbit distance
        orbitPlaneProjected = (targetPos + glm::normalize(orbitPlaneProjected - targetPos) * orbitRadius);

        // Direction towards the projected point
        const auto orbitToProjected = glm::normalize(orbitPlaneProjected - targetPos);

        // Forward direction along the orbit plane
        const auto orbitForward = glm::rotate(Vector3{0.0f, 1.0f, 0.0f}, -glm::radians(90.0f), orbitToProjected);

        targetPos = orbitPlaneProjected + (orbitForward * 100.0f);

        // Recalculate the distance
        computedDistance = glm::distance(transform.getPosition(), targetPos);
    }
    // Or should we keep at specific distance?
    else if (approachMinDistance > 1.0f) {
        // Get vector towards the target
        const auto targetToShip = glm::normalize(transform.getPosition() - targetPos);

        // Create a position that is at some specific distance from the target
        targetPos = targetPos + targetToShip * approachMinDistance;

        // Recalculate the distance
        computedDistance = glm::distance(transform.getPosition(), targetPos);
    }

    // Our current orientation (directly forward)
    const auto ourOrientation = glm::quat_cast(transform.getTransform());
    const auto ourForward = glm::rotate(ourOrientation, Vector3{0.0f, 0.0f, 1.0f});

    // Our acceleration
    const auto secondsToStop = forwardVelocity / forwardAcceleration;
    const auto distanceToStop = (1.0f / 2.0f) * forwardAcceleration * std::pow(secondsToStop, 2.0f);

    // Should we stop?
    if (approachTarget == NullEntity && !hasApproachDir) {
        targetPos = transform.getPosition() + (-ourForward * distanceToStop * 0.5f);

        // Recalculate the distance
        computedDistance = glm::distance(transform.getPosition(), targetPos);
    }

    // Fix nan if direction towards the target is equal to upVector
    const auto d = glm::dot(glm::normalize(targetPos - transform.getPosition()), upVector);
    if (glm::abs(d) > 0.99999f) {
        targetPos.x += 1.0f;
    }

    // The orientation towards the target
    Matrix4 newTransform;
    if (glm::distance2(transform.getPosition(), targetPos) == 0) {
        newTransform = glm::inverse(glm::lookAt(transform.getPosition(), targetPos + ourForward, upVector));
    } else {
        newTransform = glm::inverse(glm::lookAt(transform.getPosition(), targetPos, upVector));
    }
    newTransform = glm::rotate(newTransform, glm::radians(180.0f), Vector3{0.0f, 1.0f, 0.0f});
    auto targetOrientation = glm::quat_cast(newTransform);

    // Calculate the angle between out forward and the target direction we should be facing
    auto targetForward = glm::rotate(targetOrientation, Vector3{0.0f, 0.0f, 1.0f});
    auto angle = glm::acos(glm::dot(targetForward, ourForward));
    if (std::isnan(angle)) {
        angle = 0.0f;
    }

    // Calculate the velocity we should apply in order to approach the target or stop
    float velocityDiff;
    auto slowing{false};
    {
        // logger.info("secondsToStop: {} distanceToStop: {}", secondsToStop, distanceToStop);

        if (computedDistance > distanceToStop + extraDistanceOffset && angle < accelerationAngleMin) {
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

    // Did we arrive to the destination?
    if (computedDistance < extraDistanceOffset && approachMinDistance < 1.0f) {
        approachTarget = NullEntity;
    }

    // Align the ship horizontally if the ship is stopped
    auto aligning{false};
    if (std::abs(velocityDiff) < 1.0f && computedDistance < extraDistanceOffset) {
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

        if (angle < glm::radians(1.0f)) {
            setActive(false);
        }
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
    scene.setDirty(transform);
    scene.setDirty(*this);
}

void ComponentShipControl::actionApproach(const EntityId target) {
    actionStopMovement();
    approachTarget = target;
    setActive(true);
}

void ComponentShipControl::actionOrbit(const EntityId target, const float radius) {
    actionStopMovement();
    approachTarget = target;
    orbitRadius = radius;
    orbitMatrixChosen = false;
    setActive(true);
}

void ComponentShipControl::actionKeepDistance(EntityId target, const float distance) {
    actionStopMovement();
    approachTarget = target;
    approachMinDistance = distance;
    setActive(true);
}

void ComponentShipControl::actionStopMovement() {
    approachTarget = NullEntity;
    approachPos = Vector3{0.0f};
    approachDir = Vector3{0.0f};
    orbitRadius = 0.0f;
    approachMinDistance = 0.0f;
}

void ComponentShipControl::actionGoDirection(const Vector3& value) {
    actionStopMovement();
    approachDir = value;
    setActive(true);
}

void ComponentShipControl::actionWarpTo(const Vector3& direction) {
    actionGoDirection(direction);
}

void ComponentShipControl::setActive(const bool value) {
    active = value;
}

void ComponentShipControl::setForwardVelocityMax(const float value) {
    forwardVelocityMax = value;
}

void ComponentShipControl::setAngularVelocity(const float radians) {
    angularVelocity = radians;
}

void ComponentShipControl::addTurret(ComponentTurret& turret) {
    turrets.push_back(&turret);
}*/
