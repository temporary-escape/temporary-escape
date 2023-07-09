
#include "controller_icon.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerIcon::ControllerIcon(entt::registry& reg) : reg{reg} {
}

ControllerIcon::~ControllerIcon() = default;

void ControllerIcon::update(const float delta) {
}

void ControllerIcon::recalculate(VulkanRenderer& vulkan) {
    for (auto& [_, count] : counts) {
        count = 0;
    }

    const auto& entities = reg.view<ComponentTransform, ComponentIcon>(entt::exclude<TagDisabled>).each();
    for (const auto&& [handle, transform, component] : entities) {
        const auto& image = component.getImage();
        auto& buffer = getBufferFor(vulkan, image);

        if (counts[image.get()] > 1024) {
            continue;
        }

        auto& offset = counts[image.get()];
        auto dst = reinterpret_cast<Point*>(buffer.getCurrentBuffer().getMappedPtr()) + offset++;

        *dst = Point{transform.getAbsolutePosition(),
                     component.getSize(),
                     component.getColor(),
                     image->getAllocation().uv,
                     image->getAllocation().st,
                     component.getOffset()};
    }
}

VulkanDoubleBuffer ControllerIcon::createVbo(VulkanRenderer& vulkan) {
    VulkanBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(Point) * 1024;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
    bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    return vulkan.createDoubleBuffer(bufferInfo);
}

VulkanDoubleBuffer& ControllerIcon::getBufferFor(VulkanRenderer& vulkan, const ImagePtr& image) {
    auto it = vbos.find(image.get());
    if (it == vbos.end()) {
        it = vbos.emplace(image.get(), createVbo(vulkan)).first;
    }

    return it->second;
}
