// clang-format off
#define VMA_IMPLEMENTATION
#define VMA_VULKAN_VERSION 1001000
#include <vk_mem_alloc.h>
// clang-format on
#include "vulkan_allocator.hpp"
#include "../utils/exceptions.hpp"
#include "vulkan_device.hpp"

using namespace Engine;

VulkanAllocator::VulkanAllocator(VulkanDevice& device) {
    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_1;
    allocatorCreateInfo.physicalDevice = device.getPhysicalDevice();
    allocatorCreateInfo.device = device.getDevice();
    allocatorCreateInfo.instance = device.getInstance();

    if (vmaCreateAllocator(&allocatorCreateInfo, &allocator) != VK_SUCCESS) {
        destroy();
        EXCEPTION("Failed to create VMA allocator!");
    }
}

VulkanAllocator::~VulkanAllocator() {
    destroy();
}

VulkanAllocator::VulkanAllocator(VulkanAllocator&& other) noexcept {
    swap(other);
}

VulkanAllocator& VulkanAllocator::operator=(VulkanAllocator&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VulkanAllocator::swap(VulkanAllocator& other) noexcept {
    std::swap(allocator, other.allocator);
}

void VulkanAllocator::destroy() {
    if (allocator) {
        vmaDestroyAllocator(allocator);
        allocator = VK_NULL_HANDLE;
    }
}
