#pragma once

#include "../Controller.hpp"
#include "../DynamicsWorld.hpp"
#include "../Entity.hpp"
#include "../Pathfinding.hpp"

namespace Engine {
class ENGINE_API Scene;

class ENGINE_API ControllerPathfinding : public Controller, public Pathfinding::Tester {
public:
    explicit ControllerPathfinding(Scene& scene, entt::registry& reg, DynamicsWorld& dynamicsWorld);
    ~ControllerPathfinding() override;
    NON_COPYABLE(ControllerPathfinding);
    NON_MOVEABLE(ControllerPathfinding);

    void update(float delta) override;
    void recalculate(VulkanRenderer& vulkan) override;
    void buildTree();
    void debug(Scene& scene);

    Pathfinding& getOctree() {
        return octree;
    }

    [[nodiscard]] const Pathfinding& getOctree() const {
        return octree;
    }

    bool contactTestBox(const Vector3i& pos, int width) override;

private:
    Scene& scene;
    entt::registry& reg;
    DynamicsWorld& dynamicsWorld;
    Pathfinding octree;

    VulkanDoubleBuffer vbo;
};
} // namespace Engine
