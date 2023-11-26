#pragma once

#include "../Utils/Path.hpp"
#include "VulkanSemaphore.hpp"

namespace Engine {
class ENGINE_API VulkanDevice;

class ENGINE_API VulkanFence {
public:
    VulkanFence() = default;
    explicit VulkanFence(VulkanDevice& device);
    ~VulkanFence();
    VulkanFence(const VulkanFence& other) = delete;
    VulkanFence(VulkanFence&& other) noexcept;
    VulkanFence& operator=(const VulkanFence& other) = delete;
    VulkanFence& operator=(VulkanFence&& other) noexcept;
    void swap(VulkanFence& other) noexcept;

    void reset();
    void wait();
    bool isDone();

    VkFence& getHandle() {
        return fence;
    }

    const VkFence& getHandle() const {
        return fence;
    }

    operator bool() const {
        return fence != VK_NULL_HANDLE;
    }

    void destroy();

private:
    VkDevice device{VK_NULL_HANDLE};
    VkFence fence{VK_NULL_HANDLE};
};
} // namespace Engine
