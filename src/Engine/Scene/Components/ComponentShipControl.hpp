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

    void actionApproach(EntityId target);

    MSGPACK_DEFINE_ARRAY(/*velocityValue, velocityTarget, velocityMax, rotation, velocityBoost*/);

protected:
    void patch(entt::registry& reg, entt::entity handle) override;

private:
    /*float velocityValue{0.0f};
    float velocityTarget{0.0f};
    float velocityMax{100.0f};
    Vector3 rotation{0.0f, 0.0f, 0.0f};
    bool velocityBoost{false};
    bool directionRelative[4] = {false, false, false, false};*/
    EntityId approachTarget{NullEntity};
    Vector3 approachPos;
    float angularVelocity{glm::radians(25.0f)};
    bool active{false};
    std::vector<ComponentTurret*> turrets;
};
} // namespace Engine
