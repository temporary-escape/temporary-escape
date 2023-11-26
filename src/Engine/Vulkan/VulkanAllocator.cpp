// clang-format off
#include <cstdio>
#include <volk.h>
#define VMA_IMPLEMENTATION
#ifdef __APPLE__
#define VMA_STATS_STRING_ENABLED 0
#endif
#define VMA_VULKAN_VERSION 1001000
#include <vk_mem_alloc.h>
// clang-format on
#include "VulkanAllocator.hpp"
#include "../Utils/Exceptions.hpp"
#include "VulkanDevice.hpp"
#include <numeric>

using namespace Engine;

VulkanAllocator::VulkanAllocator(VulkanDevice& device) {
    VmaVulkanFunctions vmaVulkanFunctions{};
    vmaVulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vmaVulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_1;
    allocatorCreateInfo.physicalDevice = device.getPhysicalDevice();
    allocatorCreateInfo.device = device.getDevice();
    allocatorCreateInfo.instance = device.getInstance();
    allocatorCreateInfo.pVulkanFunctions = &vmaVulkanFunctions;

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

size_t VulkanAllocator::getUsedBytes() const {
    /*VmaTotalStatistics stats{};
    vmaCalculateStatistics(allocator, &stats);
    return stats.total.statistics.allocationBytes;*/

    std::array<VmaBudget, VK_MAX_MEMORY_HEAPS> budgets{};
    vmaGetHeapBudgets(allocator, budgets.data());
    return std::accumulate(budgets.begin(), budgets.end(), 0ULL, [](size_t i, const VmaBudget& budget) {
        return i + budget.statistics.allocationBytes;
    });
}
