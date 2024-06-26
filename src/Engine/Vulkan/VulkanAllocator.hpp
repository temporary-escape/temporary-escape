#pragma once

#include "../Utils/Path.hpp"
#include "VulkanTypes.hpp"

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

    size_t getUsedBytes() const;

    void destroy();

private:
    VmaAllocator allocator{VK_NULL_HANDLE};
};
} // namespace Engine
