#pragma once

#include "../Utils/Path.hpp"
#include "VulkanTypes.hpp"

namespace Engine {
class ENGINE_API VulkanDevice;

class ENGINE_API VulkanSemaphore : public VulkanDisposable {
public:
    using CreateInfo = VkSemaphoreCreateInfo;

    VulkanSemaphore() = default;
    explicit VulkanSemaphore(VulkanDevice& device);
    explicit VulkanSemaphore(VulkanDevice& device, const CreateInfo& createInfo);
    ~VulkanSemaphore();
    VulkanSemaphore(const VulkanSemaphore& other) = delete;
    VulkanSemaphore(VulkanSemaphore&& other) noexcept;
    VulkanSemaphore& operator=(const VulkanSemaphore& other) = delete;
    VulkanSemaphore& operator=(VulkanSemaphore&& other) noexcept;
    void swap(VulkanSemaphore& other) noexcept;

    VkSemaphore& getHandle() {
        return semaphore;
    }

    const VkSemaphore& getHandle() const {
        return semaphore;
    }

    operator bool() const {
        return semaphore != VK_NULL_HANDLE;
    }

    void destroy() override;

private:
    VkDevice device{VK_NULL_HANDLE};
    VkSemaphore semaphore{VK_NULL_HANDLE};
};
} // namespace Engine
