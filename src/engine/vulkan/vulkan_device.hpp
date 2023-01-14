#pragma once

#include "vulkan_allocator.hpp"
#include "vulkan_instance.hpp"

namespace Engine {
class ENGINE_API VulkanDevice : public VulkanInstance {
public:
    explicit VulkanDevice(const Config& config);
    virtual ~VulkanDevice();

    VkDevice& getDevice() {
        return device;
    }

    const VkDevice& getDevice() const {
        return device;
    }

    VkQueue getGraphicsQueue() const {
        return graphicsQueue;
    }

    VkQueue getPresentQueue() const {
        return presentQueue;
    }

    VkQueue getComputeQueue() const {
        return computeQueue;
    }

    VulkanAllocator& getAllocator() {
        return allocator;
    }

    const VulkanAllocator& getAllocator() const {
        return allocator;
    }

private:
    void destroy();

    const Config& config;
    VkDevice device{VK_NULL_HANDLE};
    VkQueue graphicsQueue{VK_NULL_HANDLE};
    VkQueue presentQueue{VK_NULL_HANDLE};
    VkQueue computeQueue{VK_NULL_HANDLE};
    VulkanAllocator allocator;
};
} // namespace Engine
