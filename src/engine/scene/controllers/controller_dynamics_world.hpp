#pragma once

#include "../controller.hpp"
#include "../entity.hpp"

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btIDebugDraw;

namespace Engine {
class ENGINE_API ControllerDynamicsWorld : public Controller {
public:
    explicit ControllerDynamicsWorld(entt::registry& reg);
    ~ControllerDynamicsWorld() override;

    void update(float delta) override;
    void recalculate(VulkanRenderer& vulkan) override;

    const VulkanBuffer& getDebugDrawVbo() const;
    size_t getDebugDrawCount() const;

private:
    void onConstruct(entt::registry& r, entt::entity handle);
    void onUpdate(entt::registry& r, entt::entity handle);
    void onDestroy(entt::registry& r, entt::entity handle);

    entt::registry& reg;
    std::unique_ptr<btDefaultCollisionConfiguration> collisionConfiguration;
    std::unique_ptr<btCollisionDispatcher> dispatcher;
    std::unique_ptr<btBroadphaseInterface> overlappingPairCache;
    std::unique_ptr<btSequentialImpulseConstraintSolver> solver;
    std::unique_ptr<btIDebugDraw> debugDraw;
    std::unique_ptr<btDiscreteDynamicsWorld> dynamicsWorld;
};
} // namespace Engine
