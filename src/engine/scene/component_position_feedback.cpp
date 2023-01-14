#include "component_position_feedback.hpp"

#define CMP "ComponentPositionFeedback"

using namespace Engine;

void ComponentPositionFeedback::add(const Vector3& pos) {
    setDirty(true);
    points.push_back(pos);
}

void ComponentPositionFeedback::clear() {
    setDirty(true);
    points.clear();
    points.shrink_to_fit();
}

void ComponentPositionFeedback::recalculate(VulkanRenderer& vulkan) {
    if (!isDirty()) {
        return;
    }

    setDirty(false);

    Log::d(CMP, "Recreating {} points", points.size());
    vulkan.dispose(std::move(sboInput));
    vulkan.dispose(std::move(sboOutput));

    VulkanBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(Point) * points.size();
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

    sboInput = vulkan.createBuffer(bufferInfo);
    vulkan.copyDataToBuffer(sboInput, points.data(), sizeof(Point) * points.size());

    bufferInfo = VulkanDoubleBuffer::CreateInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(Output) * points.size();
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;

    sboOutput = vulkan.createDoubleBuffer(bufferInfo);
}
