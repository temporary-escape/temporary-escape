#include "ComponentLines.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

std::vector<ComponentLines::Line> ComponentLines::createCircle(const float radius, const Color4& color) {
    std::vector<Line> res;
    res.resize(1024);

    const auto stepAngle = 360.0f / static_cast<float>(res.size());
    for (size_t i = 0; i < res.size(); i++) {
        const auto startAngle = static_cast<float>(i) * stepAngle;
        const auto endAngle = static_cast<float>(i + 1) * stepAngle;

        res[i].a = {glm::rotateY(Vector3{radius, 0.0f, 0.0f}, glm::radians(startAngle)), color};
        res[i].b = {glm::rotateY(Vector3{radius, 0.0f, 0.0f}, glm::radians(endAngle)), color};
    }

    return res;
}

ComponentLines::ComponentLines(EntityId entity, std::vector<Line> lines) :
    Component{entity}, lines{std::move(lines)}, dirty{true} {
}

void ComponentLines::recalculate(VulkanRenderer& vulkan) {
    if (!dirty) {
        return;
    }

    dirty = false;

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
