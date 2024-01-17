#pragma once

#include "../Controller.hpp"
#include "../Entity.hpp"

namespace Engine {
class ENGINE_API ControllerWorldText : public Controller {
public:
    explicit ControllerWorldText(Scene& scene, entt::registry& reg);
    ~ControllerWorldText() override;
    NON_COPYABLE(ControllerWorldText);
    NON_MOVEABLE(ControllerWorldText);

    void update(float delta) override;
    void recalculate(VulkanRenderer& vulkan) override;

private:
    Scene& scene;
    entt::registry& reg;
};
} // namespace Engine
