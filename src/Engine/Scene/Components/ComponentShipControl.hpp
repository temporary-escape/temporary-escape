#pragma once

#include "../Component.hpp"
#include "ComponentTransform.hpp"
#include "ComponentTurret.hpp"

namespace Engine {
enum class ShipAutopilotAction {
    Idle = 0,
    CancellingRotation,
    Approach,
    MoveTo,
    KeepDistance,
    Align,
    Orbit,
    Direction,
    WarpingOut,
    WarpingIn,
};
}

MSGPACK_ADD_ENUM(Engine::ShipAutopilotAction)

namespace Engine {
class ENGINE_API Scene;
class ENGINE_API ComponentShipControl : public Component {
public:
    ComponentShipControl() = default;
    explicit ComponentShipControl(EntityId entity);
    COMPONENT_DEFAULTS(ComponentShipControl);

    void update(Scene& scene, float delta, ComponentTransform& ourTransform);

    void addTurret(ComponentTurret& turret);

    const std::vector<ComponentTurret*>& getTurrets() const {
        return turrets;
    }

    void actionApproach(EntityId target);
    void actionKeepDistance(EntityId target, float distance);
    void actionOrbit(EntityId target, float radius);
    void actionCancelMovement();
    void actionMoveTo(const Vector3& pos);

    bool isReplicated() const {
        return replicated;
    }

    void setReplicated(bool value);

    MSGPACK_DEFINE_ARRAY(approachTarget, action, forwardVelocity, forwardAcceleration, forwardVelocityMax,
                         angularVelocity, targetDistance, keepAtDistance, orbitMatrix, orbitOrigin, targetPos,
                         orbitDistance);

    const Matrix4& getOrbitMatrix() const {
        return orbitMatrix;
    }

    const Vector3& getOrbitOrigin() const {
        return orbitOrigin;
    }

    const Vector3& getTargetPos() const {
        return targetPos;
    }

    float getForwardVelocity() const {
        return forwardVelocity;
    }

    float getForwardVelocityMax() const {
        return forwardVelocityMax;
    }

    EntityId getApproachEntity() const {
        return approachTarget;
    }

    ShipAutopilotAction getAction() const {
        return action;
    }

    float getTargetDistance() const {
        return targetDistance;
    }

    float getKeepAtDistance() const {
        return keepAtDistance;
    }

    float getOrbitDistance() const {
        return orbitDistance;
    }

private:
    std::optional<Vector3> getSteeringApproach(Scene& scene, float delta, ComponentTransform& ourTransform);
    std::optional<Vector3> getSteeringKeepAtDistance(Scene& scene, float delta, ComponentTransform& ourTransform);
    std::optional<Vector3> getSteeringOrbit(Scene& scene, float delta, ComponentTransform& ourTransform);
    std::optional<Vector3> getSteeringMoveTo(Scene& scene, float delta, ComponentTransform& ourTransform);

    bool replicated{false};
    EntityId approachTarget{NullEntity};
    ShipAutopilotAction action{ShipAutopilotAction::Idle};
    float forwardVelocity{0.0f};
    float forwardAcceleration{20.0f};
    float forwardVelocityMax{500.0f};
    float angularVelocity{glm::radians(45.0f)};
    float targetDistance{0.0f};
    float keepAtDistance{0.0f};
    Matrix4 orbitMatrix{1.0f};
    Vector3 orbitOrigin;
    Vector3 orbitNormal;
    Vector3 targetPos;
    bool orbitMatrixChosen{false};
    float orbitDistance{0.0f};
    float ourBounds{0.0f};
    float targetBounds{0.0f};
    bool recalculateBounds{true};

    std::vector<ComponentTurret*> turrets;
};
} // namespace Engine
