#include "vulkan_sync_object.hpp"
#include "../utils/exceptions.hpp"
#include "vulkan_device.hpp"

using namespace Engine;

VulkanSyncObject::VulkanSyncObject(VulkanDevice& device) : device{device.getDevice()} {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(device.getDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(device.getDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(device.getDevice(), &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) {

        destroy();
        EXCEPTION("Failed to create synchronization objects for a frame!");
    }
}

VulkanSyncObject::~VulkanSyncObject() {
    destroy();
}

VulkanSyncObject::VulkanSyncObject(VulkanSyncObject&& other) noexcept {
    swap(other);
}

VulkanSyncObject& VulkanSyncObject::operator=(VulkanSyncObject&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VulkanSyncObject::swap(VulkanSyncObject& other) noexcept {
    std::swap(device, other.device);
    std::swap(imageAvailableSemaphore, other.imageAvailableSemaphore);
    std::swap(renderFinishedSemaphore, other.renderFinishedSemaphore);
    std::swap(inFlightFence, other.inFlightFence);
}

void VulkanSyncObject::destroy() {
    if (imageAvailableSemaphore) {
        vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
        imageAvailableSemaphore = VK_NULL_HANDLE;
    }

    if (renderFinishedSemaphore) {
        vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
        renderFinishedSemaphore = VK_NULL_HANDLE;
    }

    if (inFlightFence) {
        vkDestroyFence(device, inFlightFence, nullptr);
        inFlightFence = VK_NULL_HANDLE;
    }
}

void VulkanSyncObject::reset() {
    vkResetFences(device, 1, &inFlightFence);
}

void VulkanSyncObject::wait() {
    vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
}