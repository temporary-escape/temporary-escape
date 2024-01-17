
#include "ControllerIcon.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerIcon::ControllerIcon(Scene& scene, entt::registry& reg) : scene{scene}, reg{reg} {
    reg.on_construct<ComponentIcon>().connect<&ControllerIcon::onConstruct>(this);
    reg.on_update<ComponentIcon>().connect<&ControllerIcon::onUpdate>(this);
    reg.on_destroy<ComponentIcon>().connect<&ControllerIcon::onDestroy>(this);
}

ControllerIcon::~ControllerIcon() {
    reg.on_construct<ComponentIcon>().disconnect<&ControllerIcon::onConstruct>(this);
    reg.on_update<ComponentIcon>().disconnect<&ControllerIcon::onUpdate>(this);
    reg.on_destroy<ComponentIcon>().disconnect<&ControllerIcon::onDestroy>(this);
}

void ControllerIcon::update(const float delta) {
}

void ControllerIcon::recalculate(VulkanRenderer& vulkan) {
    for (auto& pair : staticBuffers) {
        pair.second.recalculate(vulkan);
    }

    for (auto& pair : dynamicBuffers) {
        pair.second.clear();
    }

    const auto& entities = reg.view<ComponentTransform, ComponentIcon>(entt::exclude<TagDisabled>).each();
    for (const auto&& [handle, transform, icon] : entities) {
        if (transform.isStatic() || !icon.getImage()) {
            continue;
        }

        const auto* key = icon.getImage()->getAllocation().texture;
        auto found = dynamicBuffers.find(key);
        if (found == dynamicBuffers.end()) {
            const auto stride = sizeof(Point);

            VulkanArrayBuffer::CreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.stride = stride;
            bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO;
            bufferInfo.memoryFlags =
                VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

            found = dynamicBuffers.emplace(key, VulkanArrayBuffer{bufferInfo}).first;
        }

        auto* raw = found->second.insert(static_cast<uint64_t>(handle));
        auto* dst = reinterpret_cast<Point*>(raw);

        dst->position = transform.getAbsoluteInterpolatedPosition();
        dst->size = icon.getSize();
        dst->offset = icon.getOffset();
        dst->color = icon.getColor();
        dst->uv = icon.getImage()->getAllocation().uv;
        dst->st = icon.getImage()->getAllocation().st;
    }

    for (auto& pair : dynamicBuffers) {
        pair.second.recalculate(vulkan);
    }
}

void ControllerIcon::addOrUpdate(entt::entity handle, const ComponentTransform& transform, const ComponentIcon& icon) {
    const auto* key = icon.getImage()->getAllocation().texture;

    auto found = staticBuffers.find(key);
    if (found == staticBuffers.end()) {
        const auto stride = sizeof(Point);

        VulkanArrayBuffer::CreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.stride = stride;
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO;
        bufferInfo.memoryFlags =
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        found = staticBuffers.emplace(key, VulkanArrayBuffer{bufferInfo}).first;
    }

    auto* raw = found->second.insert(static_cast<uint64_t>(handle));
    auto* dst = reinterpret_cast<Point*>(raw);

    dst->position = transform.getAbsoluteInterpolatedPosition();
    dst->size = icon.getSize();
    dst->offset = icon.getOffset();
    dst->color = icon.getColor();
    dst->uv = icon.getImage()->getAllocation().uv;
    dst->st = icon.getImage()->getAllocation().st;
}

void ControllerIcon::remove(entt::entity handle, const ComponentIcon& icon) {
    const auto* key = icon.getImage()->getAllocation().texture;
    const auto found = staticBuffers.find(key);
    if (found != staticBuffers.end()) {
        found->second.remove(static_cast<uint64_t>(handle));
    }
}

void ControllerIcon::onConstruct(entt::registry& r, entt::entity handle) {
    const auto* transform = reg.try_get<ComponentTransform>(handle);
    if (!transform || !transform->isStatic()) {
        return;
    }
    const auto& icon = reg.get<ComponentIcon>(handle);
    if (!icon.getImage()) {
        return;
    }
    addOrUpdate(handle, *transform, icon);
}

void ControllerIcon::onUpdate(entt::registry& r, entt::entity handle) {
    const auto* transform = reg.try_get<ComponentTransform>(handle);
    if (!transform || !transform->isStatic()) {
        return;
    }
    const auto& icon = reg.get<ComponentIcon>(handle);
    if (!icon.getImage()) {
        return;
    }
    addOrUpdate(handle, *transform, icon);
}

void ControllerIcon::onDestroy(entt::registry& r, entt::entity handle) {
    const auto& icon = reg.get<ComponentIcon>(handle);
    remove(handle, icon);
}
