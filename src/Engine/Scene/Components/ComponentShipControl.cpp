#include "ComponentShipControl.hpp"
#include "../../Utils/Exceptions.hpp"
#include "../Scene.hpp"
#include <glm/gtx/euler_angles.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ComponentShipControl::ComponentShipControl(EntityId entity) : Component{entity} {
}

void ComponentShipControl::update(Scene& scene, const float delta, ComponentTransform& transform) {
    constexpr auto accelerationAngleMin = glm::radians(30.0f);
    constexpr auto slowdownAngle = glm::radians(15.0f);

    if (!active) {
        return;
    }

    // Is the target entity valid?
    if (approachTarget == entity || !scene.valid(approachTarget)) {
        actionStopMovement();
    }

    // Does the entity have transform component?
    const auto* targetTransform = scene.tryGetComponent<ComponentTransform>(approachTarget);
    if (!targetTransform) {
        actionStopMovement();
    }

    // Remember the approach position
    if (targetTransform) {
        approachPos = targetTransform->getPosition();
    }

    // Target information
    auto targetPos = approachPos;
    approachDistance = glm::distance(transform.getPosition(), approachPos);

    // Subtract target entity bounds (if it is an entity)
    if (targetTransform) {
        approachDistance -= scene.getEntityBounds(approachTarget, *targetTransform);
    }

    // Subtract our own bounds
    approachDistance -= scene.getEntityBounds(entity, transform);

    if (approachDistance < 0.0f) {
        approachDistance = 0.0f;
    }
    auto computedDistance = approachDistance;

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
    if (approachTarget == NullEntity) {
        targetPos = transform.getPosition() + (-ourForward * distanceToStop * 0.5f);

        // Recalculate the distance
        computedDistance = glm::distance(transform.getPosition(), targetPos);
    }

    // The orientation towards the target
    Matrix4 newTransform;
    if (glm::distance2(transform.getPosition(), targetPos) == 0) {
        newTransform = glm::inverse(glm::lookAt(transform.getPosition(), targetPos + ourForward, {0.0f, 1.0f, 0.0f}));
    } else {
        newTransform = glm::inverse(glm::lookAt(transform.getPosition(), targetPos, {0.0f, 1.0f, 0.0f}));
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
    approachTarget = target;
    orbitRadius = 0.0f;
    approachMinDistance = 0.0f;
    setActive(true);
}

void ComponentShipControl::actionOrbit(const EntityId target, const float radius) {
    approachTarget = target;
    orbitRadius = radius;
    approachMinDistance = 0.0f;
    orbitMatrixChosen = false;
    setActive(true);
}

void ComponentShipControl::actionKeepDistance(EntityId target, const float distance) {
    approachTarget = target;
    orbitRadius = 0.0f;
    approachMinDistance = distance;
    setActive(true);
}

void ComponentShipControl::actionStopMovement() {
    approachTarget = NullEntity;
    approachPos = Vector3{0.0f};
    orbitRadius = 0.0f;
    approachMinDistance = 0.0f;
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
}
