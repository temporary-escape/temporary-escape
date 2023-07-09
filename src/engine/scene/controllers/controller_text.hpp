#pragma once

#include "../controller.hpp"
#include "../entity.hpp"

namespace Engine {
class ENGINE_API ControllerText : public Controller {
public:
    explicit ControllerText(entt::registry& reg);
    ~ControllerText() override;
    NON_COPYABLE(ControllerText);
    NON_MOVEABLE(ControllerText);

    void update(float delta) override;
    void recalculate(VulkanRenderer& vulkan) override;

private:
    entt::registry& reg;
};
} // namespace Engine
