#include "ComponentLines.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ComponentLines::ComponentLines(entt::registry& reg, entt::entity handle, std::vector<Line> lines) :
    Component{reg, handle}, lines(std::move(lines)) {
    setDirty(true);
}

void ComponentLines::recalculate(VulkanRenderer& vulkan) {
    if (!isDirty()) {
        return;
    }

    setDirty(false);

    logger.debug("Recreating with {} lines", lines.size());

    vulkan.dispose(std::move(mesh.vbo));

    VulkanBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(Line) * lines.size();
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO;
    bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

    mesh.vbo = vulkan.createBuffer(bufferInfo);
    vulkan.copyDataToBuffer(mesh.vbo, lines.data(), bufferInfo.size);
    mesh.count = lines.size() * 2;
}
