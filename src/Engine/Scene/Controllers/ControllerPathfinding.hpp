#pragma once

#include "../Controller.hpp"
#include "../Entity.hpp"
#include "../Pathfinding.hpp"
#include "ControllerDynamicsWorld.hpp"

namespace Engine {
class ENGINE_API Scene;

class ENGINE_API ControllerPathfinding : public Controller, public Pathfinding::Tester {
public:
    explicit ControllerPathfinding(entt::registry& reg, ControllerDynamicsWorld& dynamicsWorld);
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

    bool contactTestBox(const Vector3& pos, float width) override;

private:
    entt::registry& reg;
    ControllerDynamicsWorld& dynamicsWorld;
    Pathfinding octree;

    VulkanDoubleBuffer vbo;
};
} // namespace Engine
