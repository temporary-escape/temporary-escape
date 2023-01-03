#include "vulkan_fence.hpp"
#include "../utils/exceptions.hpp"
#include "vulkan_device.hpp"

using namespace Engine;

VulkanFence::VulkanFence(VulkanDevice& device) : device{device.getDevice()} {
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateFence(device.getDevice(), &fenceInfo, nullptr, &fence) != VK_SUCCESS) {

        destroy();
        EXCEPTION("Failed to create synchronization objects for a frame!");
    }
}

VulkanFence::~VulkanFence() {
    destroy();
}

VulkanFence::VulkanFence(VulkanFence&& other) noexcept {
    swap(other);
}

VulkanFence& VulkanFence::operator=(VulkanFence&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VulkanFence::swap(VulkanFence& other) noexcept {
    std::swap(device, other.device);
    std::swap(fence, other.fence);
}

void VulkanFence::destroy() {
    if (fence) {
        vkDestroyFence(device, fence, nullptr);
        fence = VK_NULL_HANDLE;
    }
}

void VulkanFence::reset() {
    vkResetFences(device, 1, &fence);
}

void VulkanFence::wait() {
    vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
}
