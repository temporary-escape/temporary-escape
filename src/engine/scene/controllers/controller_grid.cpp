#include "controller_grid.hpp"
#include <btBulletDynamicsCommon.h>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerGrid::ControllerGrid(entt::registry& reg, ControllerDynamicsWorld& dynamicsWorld,
                               VoxelShapeCache* voxelShapeCache) :
    reg{reg}, dynamicsWorld{dynamicsWorld}, voxelShapeCache{voxelShapeCache} {

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
        if (grid.isDirty()) {
            grid.setDirty(false);
            grid.recalculate(vulkan, *voxelShapeCache);
        }
    }
}

void ControllerGrid::addOrUpdate(entt::entity handle, ComponentGrid& component) {
    logger.warn("addOrUpdate start");
    if (auto* rigidBody = reg.try_get<ComponentRigidBody>(handle); rigidBody) {
        auto shape = component.createCollisionShape();
        if (shape) {
            rigidBody->setShape(std::move(shape));
        }
    }
    logger.warn("addOrUpdate end");
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
