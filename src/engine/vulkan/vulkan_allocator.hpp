#pragma once

#include "../utils/path.hpp"
#include "vulkan_types.hpp"

namespace Engine {
class ENGINE_API VulkanDevice;

class ENGINE_API VulkanAllocator {
public:
    VulkanAllocator() = default;
    explicit VulkanAllocator(VulkanDevice& device);
    ~VulkanAllocator();
    VulkanAllocator(const VulkanAllocator& other) = delete;
    VulkanAllocator(VulkanAllocator&& other) noexcept;
    VulkanAllocator& operator=(const VulkanAllocator& other) = delete;
    VulkanAllocator& operator=(VulkanAllocator&& other) noexcept;
    void swap(VulkanAllocator& other) noexcept;

    VmaAllocator& getHandle() {
        return allocator;
    }

    void destroy();

private:
    VmaAllocator allocator{VK_NULL_HANDLE};
};
} // namespace Engine
