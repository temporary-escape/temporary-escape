#include "ControllerModel.hpp"
#include <btBulletDynamicsCommon.h>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerModel::ControllerModel(Scene& scene, entt::registry& reg) : scene{scene}, reg{reg} {
    reg.on_construct<ComponentModel>().connect<&ControllerModel::onConstruct>(this);
    reg.on_update<ComponentModel>().connect<&ControllerModel::onUpdate>(this);
    reg.on_destroy<ComponentModel>().connect<&ControllerModel::onDestroy>(this);
}

ControllerModel::~ControllerModel() {
    reg.on_construct<ComponentModel>().disconnect<&ControllerModel::onConstruct>(this);
    reg.on_update<ComponentModel>().disconnect<&ControllerModel::onUpdate>(this);
    reg.on_destroy<ComponentModel>().disconnect<&ControllerModel::onDestroy>(this);
}

void ControllerModel::update(const float delta) {
    (void)delta;
}

void ControllerModel::recalculate(VulkanRenderer& vulkan) {
    (void)vulkan;
}

void ControllerModel::addOrUpdate(entt::entity handle, ComponentModel& component) {
    auto* transform = reg.try_get<ComponentTransform>(handle);
    auto* rigidBody = reg.try_get<ComponentRigidBody>(handle);
    if (rigidBody && transform && component.getModel()) {
        transform->setScene(scene);
        rigidBody->setShape(*transform, component.getModel()->getCollisionShape().clone());
        transform->setRigidBody(*rigidBody);
    }
}

void ControllerModel::onConstruct(entt::registry& r, const entt::entity handle) {
    (void)r;
    auto& component = reg.get<ComponentModel>(handle);
    addOrUpdate(handle, component);
}

void ControllerModel::onUpdate(entt::registry& r, const entt::entity handle) {
    (void)r;
    auto& component = reg.get<ComponentModel>(handle);
    addOrUpdate(handle, component);
}

void ControllerModel::onDestroy(entt::registry& r, const entt::entity handle) {
    (void)r;
    (void)handle;
}
