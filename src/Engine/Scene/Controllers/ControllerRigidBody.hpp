#pragma once

#include "../Controller.hpp"
#include "../DynamicsWorld.hpp"

namespace Engine {
class ENGINE_API ControllerRigidBody : public Controller {
public:
    explicit ControllerRigidBody(Scene& scene, entt::registry& reg, DynamicsWorld& dynamicsWorld);
    ~ControllerRigidBody() override;
    NON_COPYABLE(ControllerRigidBody);
    NON_MOVEABLE(ControllerRigidBody);

    void update(float delta) override;
    void recalculate(VulkanRenderer& vulkan) override;

private:
    void onConstruct(entt::registry& r, entt::entity handle);
    void onDestroy(entt::registry& r, entt::entity handle);

    Scene& scene;
    entt::registry& reg;
    DynamicsWorld& dynamicsWorld;
};
} // namespace Engine
