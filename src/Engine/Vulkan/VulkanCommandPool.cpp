#include "VulkanCommandPool.hpp"
#include "../Utils/Exceptions.hpp"
#include "VulkanDevice.hpp"

using namespace Engine;

VulkanCommandPool::VulkanCommandPool(VulkanDevice& device, const CreateInfo& createInfo) : device{device.getDevice()} {
    if (vkCreateCommandPool(device.getDevice(), &createInfo, nullptr, &commandPool) != VK_SUCCESS) {
        destroy();
        EXCEPTION("Failed to create command pool!");
    }
}

VulkanCommandPool::~VulkanCommandPool() {
    destroy();
}

VulkanCommandPool::VulkanCommandPool(VulkanCommandPool&& other) noexcept {
    swap(other);
}

VulkanCommandPool& VulkanCommandPool::operator=(VulkanCommandPool&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VulkanCommandPool::swap(VulkanCommandPool& other) noexcept {
    std::swap(device, other.device);
    std::swap(commandPool, other.commandPool);
}

void VulkanCommandPool::destroy() {
    if (commandPool) {
        vkDestroyCommandPool(device, commandPool, nullptr);
        commandPool = VK_NULL_HANDLE;
    }
}
