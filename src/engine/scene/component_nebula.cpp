#include "component_nebula.hpp"

using namespace Engine;

ComponentNebula::ComponentNebula() {
    setDirty(true);
}

void ComponentNebula::recalculate(VulkanRenderer& vulkan) {
    if (!isDirty()) {
        return;
    }
    
    setDirty(false);

    if (!ubo) {
        VulkanBuffer::CreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeof(Uniform);
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
        bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
        ubo = vulkan.createBuffer(bufferInfo);
    }

    ubo.subDataLocal(&uniform, 0, sizeof(Uniform));
}
