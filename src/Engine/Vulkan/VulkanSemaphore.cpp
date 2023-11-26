#include "VulkanSemaphore.hpp"
#include "../Utils/Exceptions.hpp"
#include "VulkanDevice.hpp"

using namespace Engine;

VulkanSemaphore::VulkanSemaphore(VulkanDevice& device, const CreateInfo& createInfo) : device{device.getDevice()} {
    if (vkCreateSemaphore(device.getDevice(), &createInfo, nullptr, &semaphore) != VK_SUCCESS) {
        EXCEPTION("Failed to create semaphore!");
    }
}

VulkanSemaphore::~VulkanSemaphore() {
    destroy();
}

VulkanSemaphore::VulkanSemaphore(VulkanSemaphore&& other) noexcept {
    swap(other);
}

VulkanSemaphore& VulkanSemaphore::operator=(VulkanSemaphore&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VulkanSemaphore::swap(VulkanSemaphore& other) noexcept {
    std::swap(device, other.device);
    std::swap(semaphore, other.semaphore);
}

void VulkanSemaphore::destroy() {
    if (semaphore) {
        vkDestroySemaphore(device, semaphore, nullptr);
        semaphore = VK_NULL_HANDLE;
    }
}
