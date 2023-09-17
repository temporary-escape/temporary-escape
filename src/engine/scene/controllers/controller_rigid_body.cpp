#include "controller_rigid_body.hpp"
#include <btBulletDynamicsCommon.h>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerRigidBody::ControllerRigidBody(entt::registry& reg, ControllerDynamicsWorld& dynamicsWorld) :
    reg{reg}, dynamicsWorld{dynamicsWorld} {

    reg.on_construct<ComponentRigidBody>().connect<&ControllerRigidBody::onConstruct>(this);
    reg.on_destroy<ComponentRigidBody>().connect<&ControllerRigidBody::onDestroy>(this);
}

ControllerRigidBody::~ControllerRigidBody() {
    reg.on_construct<ComponentRigidBody>().disconnect<&ControllerRigidBody::onConstruct>(this);
    reg.on_destroy<ComponentRigidBody>().disconnect<&ControllerRigidBody::onDestroy>(this);
}

void ControllerRigidBody::update(const float delta) {
    (void)delta;
}

void ControllerRigidBody::recalculate(VulkanRenderer& vulkan) {
    (void)vulkan;
}

void ControllerRigidBody::onConstruct(entt::registry& r, const entt::entity handle) {
    auto& component = reg.get<ComponentRigidBody>(handle);
    auto rigidBody = component.getRigidBody();
    component.setDynamicsWorld(dynamicsWorld.get());
}

void ControllerRigidBody::onDestroy(entt::registry& r, const entt::entity handle) {
    auto& component = reg.get<ComponentRigidBody>(handle);
    auto rigidBody = component.getRigidBody();
    if (rigidBody) {
        dynamicsWorld.get().removeRigidBody(rigidBody);
    }
}
