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

    void update(float delta);
    void setMovement(bool left, bool right, bool up, bool down);
    [[nodiscard]] std::tuple<float, float, float> getAngles() const;

private:
    ComponentTransform* componentTransform{nullptr};
    bool moving{true};
    bool rotate[4] = {false};
};
} // namespace Engine
