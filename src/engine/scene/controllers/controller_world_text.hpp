#pragma once

#include "../controller.hpp"
#include "../entity.hpp"

namespace Engine {
class ENGINE_API ControllerWorldText : public Controller {
public:
    explicit ControllerWorldText(entt::registry& reg);
    ~ControllerWorldText() override;
    NON_COPYABLE(ControllerWorldText);
    NON_MOVEABLE(ControllerWorldText);

    void update(float delta) override;
    void recalculate(VulkanRenderer& vulkan) override;

private:
    entt::registry& reg;
};
} // namespace Engine
