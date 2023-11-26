#pragma once

#include "VulkanCommandBuffer.hpp"
#include "VulkanTypes.hpp"

namespace Engine {
class ENGINE_API VulkanCommandPool : public VulkanDisposable {
public:
    using CreateInfo = VkCommandPoolCreateInfo;

    VulkanCommandPool() = default;
    explicit VulkanCommandPool(VulkanDevice& device, const CreateInfo& createInfo);
    ~VulkanCommandPool();
    VulkanCommandPool(const VulkanCommandPool& other) = delete;
    VulkanCommandPool(VulkanCommandPool&& other) noexcept;
    VulkanCommandPool& operator=(const VulkanCommandPool& other) = delete;
    VulkanCommandPool& operator=(VulkanCommandPool&& other) noexcept;
    void swap(VulkanCommandPool& other) noexcept;

    VkCommandPool& getHandle() {
        return commandPool;
    }

    const VkCommandPool& getHandle() const {
        return commandPool;
    }

    operator bool() const {
        return getHandle() != VK_NULL_HANDLE;
    }

    void destroy() override;

private:
    VkDevice device{VK_NULL_HANDLE};
    VkCommandPool commandPool{VK_NULL_HANDLE};
};
} // namespace Engine
