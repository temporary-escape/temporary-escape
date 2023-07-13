#include "controller_static_model.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerStaticModel::ControllerStaticModel(entt::registry& reg) : reg{reg} {
    reg.on_construct<ComponentModel>().connect<&ControllerStaticModel::onConstruct>(this);
    reg.on_update<ComponentModel>().connect<&ControllerStaticModel::onUpdate>(this);
    reg.on_destroy<ComponentModel>().connect<&ControllerStaticModel::onDestroy>(this);
}

ControllerStaticModel::~ControllerStaticModel() {
    reg.on_construct<ComponentModel>().disconnect<&ControllerStaticModel::onConstruct>(this);
    reg.on_update<ComponentModel>().disconnect<&ControllerStaticModel::onUpdate>(this);
    reg.on_destroy<ComponentModel>().disconnect<&ControllerStaticModel::onDestroy>(this);
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

        VulkanArrayBuffer::CreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.stride = stride;
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO;
        bufferInfo.memoryFlags =
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        found = buffers.emplace(model.getModel().get(), VulkanArrayBuffer{bufferInfo}).first;
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
    if (!model.getModel() || !transform->isStatic()) {
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
    if (!model.getModel() || !transform->isStatic()) {
        return;
    }
    addOrUpdate(handle, *transform, model);
}

void ControllerStaticModel::onDestroy(entt::registry& r, const entt::entity handle) {
    const auto& model = reg.get<ComponentModel>(handle);
    remove(handle, model);
}
