#include "ControllerGrid.hpp"
#include <btBulletDynamicsCommon.h>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerGrid::ControllerGrid(Scene& scene, entt::registry& reg, DynamicsWorld& dynamicsWorld,
                               VoxelShapeCache* voxelShapeCache) :
    scene{scene}, reg{reg}, dynamicsWorld{dynamicsWorld}, voxelShapeCache{voxelShapeCache} {

    reg.on_construct<ComponentGrid>().connect<&ControllerGrid::onConstruct>(this);
    reg.on_update<ComponentGrid>().connect<&ControllerGrid::onUpdate>(this);
    reg.on_destroy<ComponentGrid>().connect<&ControllerGrid::onDestroy>(this);
}

ControllerGrid::~ControllerGrid() {
    reg.on_construct<ComponentGrid>().disconnect<&ControllerGrid::onConstruct>(this);
    reg.on_update<ComponentGrid>().disconnect<&ControllerGrid::onUpdate>(this);
    reg.on_destroy<ComponentGrid>().disconnect<&ControllerGrid::onDestroy>(this);
}

void ControllerGrid::update(const float delta) {
    (void)delta;
}

void ControllerGrid::recalculate(VulkanRenderer& vulkan) {
    if (!voxelShapeCache) {
        EXCEPTION("Can not recalculate grid, voxel shape grid is null");
    }

    for (auto&& [_, grid] : reg.view<ComponentGrid>().each()) {
        grid.recalculate(vulkan, *voxelShapeCache);
    }
}

void ControllerGrid::addOrUpdate(entt::entity handle, ComponentGrid& component) {
    auto* transform = reg.try_get<ComponentTransform>(handle);
    auto* rigidBody = reg.try_get<ComponentRigidBody>(handle);
    if (rigidBody && transform) {
        auto shape = component.createCollisionShape();
        if (shape) {
            transform->setScene(scene);
            rigidBody->setShape(*transform, std::move(shape));
            transform->setRigidBody(*rigidBody);
        }
    }
}

void ControllerGrid::onConstruct(entt::registry& r, entt::entity handle) {
    (void)r;
    (void)handle;
}

void ControllerGrid::onUpdate(entt::registry& r, entt::entity handle) {
    (void)r;
    auto& component = reg.get<ComponentGrid>(handle);
    addOrUpdate(handle, component);
}

void ControllerGrid::onDestroy(entt::registry& r, entt::entity handle) {
    (void)r;
    (void)handle;
}
