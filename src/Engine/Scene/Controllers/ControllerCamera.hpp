#pragma once

#include "../Controller.hpp"
#include "../Entity.hpp"

namespace Engine {
class ENGINE_API ControllerCamera : public Controller {
public:
    explicit ControllerCamera(Scene& scene, entt::registry& reg);
    ~ControllerCamera() override;
    NON_COPYABLE(ControllerCamera);
    NON_MOVEABLE(ControllerCamera);

    void update(float delta) override;
    void recalculate(VulkanRenderer& vulkan) override;

private:
    Scene& scene;
    entt::registry& reg;
};
} // namespace Engine
