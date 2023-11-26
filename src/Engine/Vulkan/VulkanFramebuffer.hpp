#pragma once

#include "../Utils/Path.hpp"
#include "VulkanTypes.hpp"

namespace Engine {
class ENGINE_API VulkanDevice;

class ENGINE_API VulkanFramebuffer : public VulkanDisposable {
public:
    using CreateInfo = VkFramebufferCreateInfo;

    VulkanFramebuffer() = default;
    explicit VulkanFramebuffer(VulkanDevice& device, const CreateInfo& createInfo);
    ~VulkanFramebuffer();
    VulkanFramebuffer(const VulkanFramebuffer& other) = delete;
    VulkanFramebuffer(VulkanFramebuffer&& other) noexcept;
    VulkanFramebuffer& operator=(const VulkanFramebuffer& other) = delete;
    VulkanFramebuffer& operator=(VulkanFramebuffer&& other) noexcept;
    void swap(VulkanFramebuffer& other) noexcept;

    VkFramebuffer& getHandle() {
        return framebuffer;
    }

    const VkFramebuffer& getHandle() const {
        return framebuffer;
    }

    operator bool() const {
        return framebuffer != VK_NULL_HANDLE;
    }

    void destroy() override;

private:
    VkDevice device{VK_NULL_HANDLE};
    VkFramebuffer framebuffer{VK_NULL_HANDLE};
};
} // namespace Engine
