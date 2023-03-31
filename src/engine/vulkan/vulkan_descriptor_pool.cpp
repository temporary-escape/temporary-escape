#include "vulkan_descriptor_pool.hpp"
#include "../utils/exceptions.hpp"
#include "vulkan_renderer.hpp"

using namespace Engine;

VulkanDescriptorPool::VulkanDescriptorPool(VulkanDevice& device) : device{device.getDevice()} {

    std::array<VkDescriptorPoolSize, 3> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = /*static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)*/ 256;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = /*static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)*/ 256;
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[2].descriptorCount = /*static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)*/ 256;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = /*static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) **/ 256;
    // poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    if (vkCreateDescriptorPool(device.getDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        destroy();
        EXCEPTION("Failed to create descriptor pool!");
    }
}

VulkanDescriptorPool::VulkanDescriptorPool(VulkanDevice& device, const CreateInfo& createInfo) :
    device{device.getDevice()} {
    if (vkCreateDescriptorPool(device.getDevice(), &createInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        destroy();
        EXCEPTION("Failed to create descriptor pool!");
    }
}

VulkanDescriptorPool::~VulkanDescriptorPool() {
    destroy();
}

VulkanDescriptorPool::VulkanDescriptorPool(VulkanDescriptorPool&& other) noexcept {
    swap(other);
}

VulkanDescriptorPool& VulkanDescriptorPool::operator=(VulkanDescriptorPool&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VulkanDescriptorPool::swap(VulkanDescriptorPool& other) noexcept {
    std::swap(device, other.device);
    std::swap(descriptorPool, other.descriptorPool);
}

void VulkanDescriptorPool::destroy() {
    if (descriptorPool) {
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        descriptorPool = VK_NULL_HANDLE;
    }
}

void VulkanDescriptorPool::reset() {
    vkResetDescriptorPool(device, descriptorPool, 0);
}
