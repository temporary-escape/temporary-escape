#pragma once

#include "../DynamicsWorld.hpp"
#include "ControllerBullets.hpp"

namespace Engine {
class ENGINE_API ControllerTurret : public Controller {
public:
    explicit ControllerTurret(Scene& scene, entt::registry& reg, DynamicsWorld& dynamicsWorld,
                              ControllerBullets& bullets);
    ~ControllerTurret() override;
    NON_COPYABLE(ControllerTurret);
    NON_MOVEABLE(ControllerTurret);

    void update(float delta) override;

    void recalculate(VulkanRenderer& vulkan) override;

private:
    void onDestroyTransform(entt::registry& r, entt::entity handle);

    Scene& scene;
    entt::registry& reg;
    DynamicsWorld& dynamicsWorld;
    ControllerBullets& bullets;
};
} // namespace Engine
