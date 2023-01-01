#pragma once

#include "../utils/path.hpp"
#include "vulkan_semaphore.hpp"

namespace Engine {
class ENGINE_API VulkanDevice;

class ENGINE_API VulkanSyncObject {
public:
    VulkanSyncObject() = default;
    explicit VulkanSyncObject(VulkanDevice& device);
    ~VulkanSyncObject();
    VulkanSyncObject(const VulkanSyncObject& other) = delete;
    VulkanSyncObject(VulkanSyncObject&& other) noexcept;
    VulkanSyncObject& operator=(const VulkanSyncObject& other) = delete;
    VulkanSyncObject& operator=(VulkanSyncObject&& other) noexcept;
    void swap(VulkanSyncObject& other) noexcept;

    void reset();
    void wait();

    const VulkanSemaphore& getImageAvailableSemaphore() const {
        return imageAvailableSemaphore;
    }

    const VulkanSemaphore& getRenderFinishedSemaphore() const {
        return renderFinishedSemaphore;
    }

    VkFence& getHandle() {
        return inFlightFence;
    }

    const VkFence& getHandle() const {
        return inFlightFence;
    }

    operator bool() const {
        return inFlightFence != VK_NULL_HANDLE;
    }

    void destroy();

private:
    VkDevice device{VK_NULL_HANDLE};
    VulkanSemaphore imageAvailableSemaphore;
    VulkanSemaphore renderFinishedSemaphore;
    VkFence inFlightFence{VK_NULL_HANDLE};
};
} // namespace Engine
