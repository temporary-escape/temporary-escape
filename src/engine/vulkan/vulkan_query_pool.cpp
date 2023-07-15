#include "vulkan_query_pool.hpp"
#include "../utils/exceptions.hpp"
#include "vulkan_renderer.hpp"

using namespace Engine;

VulkanQueryPool::VulkanQueryPool(VulkanRenderer& device, const VulkanQueryPool::CreateInfo& createInfo) :
    device{&device} {

    for (auto& pool : pools) {
        if (vkCreateQueryPool(device.getDevice(), &createInfo, nullptr, &pool) != VK_SUCCESS) {
            EXCEPTION("Failed to create query pool");
        }
    }
}

VulkanQueryPool::~VulkanQueryPool() {
    destroy();
}

VulkanQueryPool::VulkanQueryPool(VulkanQueryPool&& other) noexcept {
    swap(other);
}

VulkanQueryPool& VulkanQueryPool::operator=(VulkanQueryPool&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VulkanQueryPool::swap(VulkanQueryPool& other) noexcept {
    std::swap(device, other.device);
    std::swap(pools, other.pools);
}

void VulkanQueryPool::destroy() {
    if (device) {
        for (auto pool : pools) {
            if (pool != VK_NULL_HANDLE) {
                vkDestroyQueryPool(device->getDevice(), pool, nullptr);
            }
        }
    }
}

VkQueryPool& VulkanQueryPool::getHandle() {
    return pools.at(device->getCurrentFrameNum());
}

const VkQueryPool& VulkanQueryPool::getHandle() const {
    return pools.at(device->getCurrentFrameNum());
}

template <>
std::vector<uint64_t> VulkanQueryPool::getResult<uint64_t>(const uint32_t firstQuery, const uint32_t count,
                                                           const VkQueryResultFlags flags) {
    if (device) {
        EXCEPTION("Query pool not initialized");
    }

    std::vector<uint64_t> values;
    values.resize(count);

    const auto res = vkGetQueryPoolResults(device->getDevice(),
                                           getHandle(),
                                           firstQuery,
                                           count,
                                           sizeof(uint64_t) * count,
                                           values.data(),
                                           sizeof(uint64_t),
                                           flags);

    if (res == VK_NOT_READY) {
        return {};
    }

    if (res == VK_SUCCESS) {
        return values;
    }

    return {};
}
