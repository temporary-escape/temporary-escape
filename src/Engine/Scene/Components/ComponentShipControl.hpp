#pragma once

#include "../Component.hpp"
#include "ComponentTransform.hpp"
#include "ComponentTurret.hpp"

namespace Engine {
class ENGINE_API ComponentShipControl : public Component {
public:
    ComponentShipControl() = default;
    explicit ComponentShipControl(entt::registry& reg, entt::entity handle);
    virtual ~ComponentShipControl() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentShipControl);

    void update(EntityRegistry& reg, float delta, ComponentTransform& transform);

    void setActive(bool value);
    [[nodiscard]] bool isActive() const {
        return active;
    }

    void addTurret(ComponentTurret& turret);
    const std::vector<ComponentTurret*>& getTurrets() const {
        return turrets;
    }

    const Matrix4& getOrbitMatrix() const {
        return orbitMatrix;
    }

    const Vector3& getOrbitOrigin() const {
        return orbitOrigin;
    }

    float getOrbitRadius() const {
        return orbitRadius;
    }

    float getApproachDistance() const {
        return approachDistance;
    }

    float getApproachMinDistance() const {
        return approachMinDistance;
    }

    const Vector3& getApproachPos() const {
        return approachPos;
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

    void setForwardVelocityMax(float value);
    void setAngularVelocity(float radians);

    void actionApproach(EntityId target);
    void actionOrbit(EntityId target, float radius);
    void actionKeepDistance(EntityId target, float distance);
    void actionStopMovement();

    MSGPACK_DEFINE_ARRAY(approachTarget, approachMinDistance, approachDistance, forwardVelocity, forwardVelocityMax,
                         orbitRadius, orbitMatrix, orbitOrigin, approachPos);

protected:
    void patch(entt::registry& reg, entt::entity handle) override;

private:
    EntityId approachTarget{NullEntity};
    float approachMinDistance{0.0f};
    float approachDistance{0.0f};
    float angularVelocity{glm::radians(45.0f)};
    float forwardVelocity{0.0f};
    float forwardVelocityTarget{0.0f};
    float forwardVelocityMax{200.0f};
    float forwardAcceleration{20.0f};
    float extraDistanceOffset{10.0f};
    float orbitRadius{0.0f};
    Matrix4 orbitMatrix{1.0f};
    Vector3 orbitOrigin;
    bool orbitMatrixChosen{false};
    bool active{false};
    Vector3 approachPos;
    std::vector<ComponentTurret*> turrets;
};
} // namespace Engine
