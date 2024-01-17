#pragma once

#include "ControllerDynamicsWorld.hpp"

namespace Engine {
class ENGINE_API ControllerGrid : public Controller {
public:
    explicit ControllerGrid(Scene& scene, entt::registry& reg, ControllerDynamicsWorld& dynamicsWorld,
                            VoxelShapeCache* voxelShapeCache);
    ~ControllerGrid() override;
    NON_COPYABLE(ControllerGrid);
    NON_MOVEABLE(ControllerGrid);

    void update(float delta) override;
    void recalculate(VulkanRenderer& vulkan) override;

private:
    void addOrUpdate(entt::entity handle, ComponentGrid& component);
    void onConstruct(entt::registry& r, entt::entity handle);
    void onUpdate(entt::registry& r, entt::entity handle);
    void onDestroy(entt::registry& r, entt::entity handle);

    Scene& scene;
    entt::registry& reg;
    ControllerDynamicsWorld& dynamicsWorld;
    VoxelShapeCache* voxelShapeCache;
};
} // namespace Engine
