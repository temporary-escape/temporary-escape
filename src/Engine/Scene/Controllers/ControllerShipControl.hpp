#pragma once

#include "../Controller.hpp"
#include "../Entity.hpp"

namespace Engine {
class ENGINE_API ControllerShipControl : public Controller {
public:
    explicit ControllerShipControl(entt::registry& reg);
    ~ControllerShipControl() override;
    NON_COPYABLE(ControllerShipControl);
    NON_MOVEABLE(ControllerShipControl);

    void update(float delta) override;

    void recalculate(VulkanRenderer& vulkan) override;

private:
    entt::registry& reg;
};
} // namespace Engine
