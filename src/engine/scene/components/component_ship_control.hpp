#pragma once

#include "../component.hpp"
#include "component_transform.hpp"

namespace Engine {
class ENGINE_API ComponentShipControl : public Component {
public:
    ComponentShipControl() = default;
    explicit ComponentShipControl(entt::registry& reg, entt::entity handle);
    virtual ~ComponentShipControl() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentShipControl);

    void update(float delta, ComponentTransform& transform);
    void setSpeed(float value);
    void setDirection(const Vector3& value);
    void setDirectionRelative(int leftRight, int downUp);
    [[nodiscard]] float getSpeed() const {
        return velocityValue;
    }

    MSGPACK_DEFINE_ARRAY(velocityValue, velocityTarget, rotation);

protected:
    void patch(entt::registry& reg, entt::entity handle) override;

private:
    float velocityValue{0.0f};
    float velocityTarget{0.0f};
    Vector3 rotation{0.0f, 0.0f, 0.0f};
    bool directionRelative[4] = {false, false, false, false};
};
} // namespace Engine
