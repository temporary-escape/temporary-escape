#include "controller_static_model.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerStaticModel::ControllerStaticModel(entt::registry& reg) : reg{reg} {
    reg.on_construct<ComponentModel>().connect<&ControllerStaticModel::onConstruct>(this);
    reg.on_update<ComponentModel>().connect<&ControllerStaticModel::onUpdate>(this);
    reg.on_destroy<ComponentModel>().connect<&ControllerStaticModel::onDestroy>(this);
}

ControllerStaticModel::~ControllerStaticModel() {
    reg.on_construct<ComponentModel>().connect<&ControllerStaticModel::onConstruct>(this);
    reg.on_update<ComponentModel>().connect<&ControllerStaticModel::onUpdate>(this);
    reg.on_destroy<ComponentModel>().connect<&ControllerStaticModel::onDestroy>(this);
}

void ControllerStaticModel::update(const float delta) {
    (void)delta;
}

void ControllerStaticModel::recalculate(VulkanRenderer& vulkan) {
    for (auto& pair : buffers) {
        pair.second.recalculate(vulkan);
    }
}

void ControllerStaticModel::addOrUpdate(entt::entity handle, const ComponentTransform& transform,
                                        const ComponentModel& model) {
    auto found = buffers.find(model.getModel().get());
    if (found == buffers.end()) {
        const auto stride = sizeof(ComponentModel::InstancedVertex);
        found = buffers.emplace(model.getModel().get(), VulkanArrayBuffer{stride}).first;
    }

    auto* raw = found->second.insert(static_cast<uint64_t>(handle));
    auto* dst = reinterpret_cast<ComponentModel::InstancedVertex*>(raw);
    dst->modelMatrix = transform.getAbsoluteTransform();
    dst->entityColor = entityColor(handle);
}

void ControllerStaticModel::remove(entt::entity handle, const ComponentModel& model) {
    const auto found = buffers.find(model.getModel().get());
    if (found != buffers.end()) {
        found->second.remove(static_cast<uint64_t>(handle));
    }
}

void ControllerStaticModel::onUpdate(entt::registry& r, entt::entity handle) {
    const auto* transform = reg.try_get<ComponentTransform>(handle);
    if (!transform) {
        return;
    }
    const auto& model = reg.get<ComponentModel>(handle);
    if (!model.getModel().get() || !model.isStatic()) {
        return;
    }
    addOrUpdate(handle, *transform, model);
}

void ControllerStaticModel::onConstruct(entt::registry& r, const entt::entity handle) {
    const auto* transform = reg.try_get<ComponentTransform>(handle);
    if (!transform) {
        return;
    }
    const auto& model = reg.get<ComponentModel>(handle);
    if (!model.getModel().get() || !model.isStatic()) {
        return;
    }
    addOrUpdate(handle, *transform, model);
}

void ControllerStaticModel::onDestroy(entt::registry& r, const entt::entity handle) {
    const auto& model = reg.get<ComponentModel>(handle);
    remove(handle, model);
}
