#include "ComponentCamera.hpp"
#include "ComponentTransform.hpp"

using namespace Engine;

// TODO
static const Config config{};

ComponentCamera::ComponentCamera(EntityId entity, ComponentTransform& transform) :
    Component{entity}, Camera{transform.getTransform()} {
}

void ComponentCamera::update(const float delta) {
}

void ComponentCamera::recalculate(VulkanRenderer& vulkan, VulkanDescriptorSetPool& descriptorPool) {
    device = &vulkan;

    auto uniform = createUniform(false);

    if (!ubo) {
        VulkanBuffer::CreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeof(Uniform);
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        bufferInfo.memoryFlags =
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        ubo = vulkan.createDoubleBuffer(bufferInfo);

        for (size_t i = 0; i < descriptorSets.size(); i++) {
            vulkan.dispose(std::move(descriptorSets[i]));
            descriptorSets[i] = descriptorPool.createDescriptorSet();
            descriptorSets[i].bindUniform(0, ubo.getBuffers()[i]);
        }
    }

    ubo.subDataLocal(&uniform, 0, sizeof(Uniform));

    uniform = createUniform(true);

    if (!uboZeroPos) {
        VulkanBuffer::CreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeof(Uniform);
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        bufferInfo.memoryFlags =
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        uboZeroPos = vulkan.createDoubleBuffer(bufferInfo);

        for (size_t i = 0; i < descriptorSetsZeroPos.size(); i++) {
            vulkan.dispose(std::move(descriptorSetsZeroPos[i]));
            descriptorSetsZeroPos[i] = descriptorPool.createDescriptorSet();
            descriptorSetsZeroPos[i].bindUniform(0, ubo.getBuffers()[i]);
        }
    }

    uboZeroPos.subDataLocal(&uniform, 0, sizeof(Uniform));
}
