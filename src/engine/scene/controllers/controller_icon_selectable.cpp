
#include "controller_icon_selectable.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerIconSelectable::ControllerIconSelectable(entt::registry& reg) : reg{reg} {
    reg.on_construct<ComponentIcon>().connect<&ControllerIconSelectable::onConstruct>(this);
    reg.on_update<ComponentIcon>().connect<&ControllerIconSelectable::onUpdate>(this);
    reg.on_destroy<ComponentIcon>().connect<&ControllerIconSelectable::onDestroy>(this);
}

ControllerIconSelectable::~ControllerIconSelectable() {
    reg.on_construct<ComponentIcon>().disconnect<&ControllerIconSelectable::onConstruct>(this);
    reg.on_update<ComponentIcon>().disconnect<&ControllerIconSelectable::onUpdate>(this);
    reg.on_destroy<ComponentIcon>().disconnect<&ControllerIconSelectable::onDestroy>(this);
}

void ControllerIconSelectable::update(const float delta) {
    (void)delta;
}

void ControllerIconSelectable::prepareOutput(VulkanRenderer& vulkan, VulkanDoubleBuffer& buffer, size_t count) {
    auto bufferInfo = VulkanDoubleBuffer::CreateInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(Output) * count;
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
    bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    logger.debug("Resizing output buffer to size: {} bytes", bufferInfo.size);

    buffer = vulkan.createDoubleBuffer(bufferInfo);
}

void ControllerIconSelectable::recalculate(VulkanRenderer& vulkan) {
    staticBufferInput.recalculate(vulkan);

    if (!dynamicBufferInput) {
        VulkanArrayBuffer::CreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.stride = sizeof(Input);
        bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO;
        bufferInfo.memoryFlags =
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        dynamicBufferInput = VulkanArrayBuffer{bufferInfo};
    }

    dynamicBufferInput.clear();

    const auto& entities = reg.view<ComponentTransform, ComponentIcon>(entt::exclude<TagDisabled>).each();
    for (const auto&& [handle, transform, icon] : entities) {
        if (transform.isStatic() || !icon.getImage() || !icon.isSelectable()) {
            continue;
        }

        auto* dst = reinterpret_cast<Input*>(dynamicBufferInput.insert(static_cast<uint64_t>(handle)));
        dst->position = Vector4{transform.getAbsolutePosition(), 1.0f};
        dst->size = icon.getSize();
        dst->id = static_cast<uint32_t>(handle);
    }

    dynamicBufferInput.recalculate(vulkan);
    staticBufferInput.recalculate(vulkan);

    if (!staticBufferInput.empty() && staticBufferOutput.getSize() / sizeof(Output) != staticBufferInput.capacity()) {
        prepareOutput(vulkan, staticBufferOutput, staticBufferInput.capacity());
    }
    if (!dynamicBufferInput.empty() &&
        dynamicBufferOutput.getSize() / sizeof(Output) != dynamicBufferInput.capacity()) {
        prepareOutput(vulkan, dynamicBufferOutput, dynamicBufferInput.capacity());
    }

    // TODO: Optimize findNearestPoint

    std::array<Output, 2> results{};
    size_t count{0};

    if (dynamicBufferOutput) {
        const auto found = findNearestPoint(dynamicBufferOutput.getPreviousBuffer(), dynamicBufferInput.count());
        if (found) {
            results[count++] = *found;
        }
    }
    if (staticBufferOutput) {
        const auto found = findNearestPoint(staticBufferOutput.getPreviousBuffer(), staticBufferInput.count());
        if (found) {
            results[count++] = *found;
        }
    }

    const Output* selected{nullptr};
    for (size_t i = 0; i < count; i++) {
        if (!selected || glm::distance(Vector2{results[i].position}, mousePos) <
                             glm::distance(Vector2{selected->position}, mousePos)) {
            auto test = entt::entity{results[i].id};
            if (reg.valid(test)) {
                /*auto icon = reg.try_get<ComponentIcon>(test);
                if (icon->getColor().a != 0.0f) {
                    selected = &results[i];
                }*/
                selected = &results[i];
            }
        }
    }

    if (selected) {
        selectedEntity = entt::entity{selected->id};
    } else {
        selectedEntity = std::nullopt;
    }
}

std::optional<ControllerIconSelectable::Output> ControllerIconSelectable::findNearestPoint(VulkanBuffer& buffer,
                                                                                           const size_t count) {
    if (count == 0) {
        return std::nullopt;
    }

    // This may look stupid, but it will save us memory access time.
    // This will make it several times orders of magnitude faster!
    std::vector<std::byte> copy;
    copy.resize(count * sizeof(Output));
    std::memcpy(copy.data(), buffer.getMappedPtr(), copy.size());

    const auto* src = reinterpret_cast<const Output*>(copy.data());
    if (!src) {
        EXCEPTION("The buffer is not mapped");
    }

    if (count == 0) {
        return std::nullopt;
    }

    const Output* selected{nullptr};

    for (size_t i = 0; i < count; i++) {
        const auto size = src[i].size / 2.0f;
        if (mousePos.x > src[i].position.x - size.x && mousePos.x < src[i].position.x + size.x &&
            mousePos.y > src[i].position.y - size.y && mousePos.y < src[i].position.y + size.y) {

            if (!selected || glm::distance(Vector2{src[i].position}, mousePos) <
                                 glm::distance(Vector2{selected->position}, mousePos)) {
                selected = &src[i];
            }
        }
    }

    if (selected) {
        return *selected;
    }
    return std::nullopt;
}

void ControllerIconSelectable::addOrUpdate(entt::entity handle, const ComponentTransform& transform,
                                           const ComponentIcon& icon) {
    if (!staticBufferInput) {
        VulkanArrayBuffer::CreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.stride = sizeof(Input);
        bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO;
        bufferInfo.memoryFlags =
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        staticBufferInput = VulkanArrayBuffer{bufferInfo};
    }

    auto* dst = reinterpret_cast<Input*>(staticBufferInput.insert(static_cast<uint64_t>(handle)));
    dst->position = Vector4{transform.getAbsolutePosition(), 1.0f};
    dst->size = icon.getSize();
    dst->id = static_cast<uint32_t>(handle);
}

void ControllerIconSelectable::remove(entt::entity handle) {
    staticBufferInput.remove(static_cast<uint64_t>(handle));
}

void ControllerIconSelectable::onConstruct(entt::registry& r, entt::entity handle) {
    const auto* transform = reg.try_get<ComponentTransform>(handle);
    if (!transform || !transform->isStatic()) {
        return;
    }
    const auto& icon = reg.get<ComponentIcon>(handle);
    if (!icon.getImage() || !icon.isSelectable()) {
        return;
    }
    addOrUpdate(handle, *transform, icon);
}

void ControllerIconSelectable::onUpdate(entt::registry& r, entt::entity handle) {
    const auto* transform = reg.try_get<ComponentTransform>(handle);
    if (!transform || !transform->isStatic()) {
        return;
    }
    const auto& icon = reg.get<ComponentIcon>(handle);
    if (!icon.getImage() || !icon.isSelectable()) {
        return;
    }
    addOrUpdate(handle, *transform, icon);
}

void ControllerIconSelectable::onDestroy(entt::registry& r, entt::entity handle) {
    remove(handle);
}

void ControllerIconSelectable::eventMouseMoved(const Vector2i& pos) {
    mousePos = pos;
}

void ControllerIconSelectable::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    (void)pos;
    (void)button;
}

void ControllerIconSelectable::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    (void)pos;
    (void)button;
}

void ControllerIconSelectable::eventMouseScroll(const int xscroll, const int yscroll) {
    (void)xscroll;
    (void)yscroll;
}

void ControllerIconSelectable::eventKeyPressed(const Key key, const Modifiers modifiers) {
    (void)key;
    (void)modifiers;
}

void ControllerIconSelectable::eventKeyReleased(const Key key, const Modifiers modifiers) {
    (void)key;
    (void)modifiers;
}

void ControllerIconSelectable::eventCharTyped(const uint32_t code) {
    (void)code;
}
