// clang-format off
#define VMA_IMPLEMENTATION
#define VMA_VULKAN_VERSION 1001000
#include <vk_mem_alloc.h>
// clang-format on
#include "vg_allocator.hpp"
#include "../utils/exceptions.hpp"
#include "vg_device.hpp"

using namespace Engine;

VgAllocator::VgAllocator(const Config& config, VgDevice& device) {
    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_1;
    allocatorCreateInfo.physicalDevice = device.getPhysicalDevice();
    allocatorCreateInfo.device = device.getHandle();
    allocatorCreateInfo.instance = device.getInstance();

    if (vmaCreateAllocator(&allocatorCreateInfo, &allocator) != VK_SUCCESS) {
        destroy();
        EXCEPTION("Failed to create VMA allocator!");
    }
}

VgAllocator::~VgAllocator() {
    destroy();
}

VgAllocator::VgAllocator(VgAllocator&& other) noexcept {
    swap(other);
}

VgAllocator& VgAllocator::operator=(VgAllocator&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VgAllocator::swap(VgAllocator& other) noexcept {
    std::swap(allocator, other.allocator);
}

void VgAllocator::destroy() {
    if (allocator) {
        vmaDestroyAllocator(allocator);
        allocator = VK_NULL_HANDLE;
    }
}
