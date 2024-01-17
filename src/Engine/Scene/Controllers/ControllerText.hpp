#pragma once

#include "../Controller.hpp"
#include "../Entity.hpp"

namespace Engine {
class ENGINE_API ControllerText : public Controller {
public:
    explicit ControllerText(Scene& scene, entt::registry& reg);
    ~ControllerText() override;
    NON_COPYABLE(ControllerText);
    NON_MOVEABLE(ControllerText);

    void update(float delta) override;
    void recalculate(VulkanRenderer& vulkan) override;

private:
    Scene& scene;
    entt::registry& reg;
};
} // namespace Engine
