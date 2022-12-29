#include "vg_descriptor_pool.hpp"
#include "../utils/exceptions.hpp"
#include "vg_device.hpp"

using namespace Engine;

VgDescriptorPool::VgDescriptorPool(const Config& config, VgDevice& device) : config{&config}, device{&device} {

    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = /*static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)*/ 4;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = /*static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)*/ 4;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = /*static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) **/ 64;
    // poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    if (vkCreateDescriptorPool(device.getHandle(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        destroy();
        EXCEPTION("Failed to create descriptor pool!");
    }
}

VgDescriptorPool::~VgDescriptorPool() {
    destroy();
}

VgDescriptorPool::VgDescriptorPool(VgDescriptorPool&& other) noexcept {
    swap(other);
}

VgDescriptorPool& VgDescriptorPool::operator=(VgDescriptorPool&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VgDescriptorPool::swap(VgDescriptorPool& other) noexcept {
    std::swap(config, other.config);
    std::swap(device, other.device);
    std::swap(descriptorPool, other.descriptorPool);
}

void VgDescriptorPool::destroy() {
    if (descriptorPool) {
        vkDestroyDescriptorPool(device->getHandle(), descriptorPool, nullptr);
        descriptorPool = VK_NULL_HANDLE;
    }
}

void VgDescriptorPool::reset() {
    vkResetDescriptorPool(device->getHandle(), descriptorPool, 0);
}
