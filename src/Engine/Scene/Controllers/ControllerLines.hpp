#pragma once

#include "../Controller.hpp"
#include "../Entity.hpp"

namespace Engine {
class ENGINE_API ControllerLines : public Controller {
public:
    explicit ControllerLines(entt::registry& reg);
    ~ControllerLines() override;
    NON_COPYABLE(ControllerLines);
    NON_MOVEABLE(ControllerLines);

    void update(float delta) override;
    void recalculate(VulkanRenderer& vulkan) override;

private:
    entt::registry& reg;
};
} // namespace Engine
