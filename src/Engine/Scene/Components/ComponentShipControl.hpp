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
    /*void setDirection(const Vector3& value);
    void setDirectionRelative(int leftRight, int downUp);
    void setSpeed(float value);
    [[nodiscard]] float getSpeed() const {
        return velocityValue;
    }
    void setSpeedMax(float value);
    [[nodiscard]] float getSpeedMax() const {
        return velocityMax;
    }
    void setSpeedBoost(bool value);
    [[nodiscard]] bool getSpeedBoost() const {
        return velocityBoost;
    }*/
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

    const Vector3& getApproachPos() const {
        return targetPos;
    }

    void actionApproach(EntityId target);
    void actionOrbit(EntityId target, float radius);

    MSGPACK_DEFINE_ARRAY(orbitMatrix, orbitRadius, orbitOrigin);

protected:
    void patch(entt::registry& reg, entt::entity handle) override;

private:
    Vector3 getTargetPos(EntityRegistry& reg);

    /*float velocityValue{0.0f};
    float velocityTarget{0.0f};
    float velocityMax{100.0f};
    Vector3 rotation{0.0f, 0.0f, 0.0f};
    bool velocityBoost{false};
    bool directionRelative[4] = {false, false, false, false};*/
    EntityId approachTarget{NullEntity};
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
    Vector3 targetPos;
    std::vector<ComponentTurret*> turrets;
};
} // namespace Engine
