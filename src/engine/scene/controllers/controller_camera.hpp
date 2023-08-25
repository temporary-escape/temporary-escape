#pragma once

#include "../controller.hpp"
#include "../entity.hpp"

namespace Engine {
class ENGINE_API ControllerCamera : public Controller {
public:
    explicit ControllerCamera(entt::registry& reg);
    ~ControllerCamera() override;
    NON_COPYABLE(ControllerCamera);
    NON_MOVEABLE(ControllerCamera);

    void update(float delta) override;
    void recalculate(VulkanRenderer& vulkan) override;

private:
    entt::registry& reg;
};
} // namespace Engine
