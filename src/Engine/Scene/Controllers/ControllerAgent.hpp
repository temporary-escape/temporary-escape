#pragma once

#include "../Controller.hpp"
#include "../Entity.hpp"

namespace Engine {
class ENGINE_API ControllerAgent : public Controller {
public:
    explicit ControllerAgent(Scene& scene, entt::registry& reg);
    ~ControllerAgent() override;
    NON_COPYABLE(ControllerAgent);
    NON_MOVEABLE(ControllerAgent);

    void update(float delta) override;
    void recalculate(VulkanRenderer& vulkan) override;

private:
    Scene& scene;
    entt::registry& reg;
    std::mt19937_64 rng;
};
} // namespace Engine
