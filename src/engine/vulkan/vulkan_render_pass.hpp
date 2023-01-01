#pragma once

#include "../utils/path.hpp"
#include "vulkan_types.hpp"

namespace Engine {
class ENGINE_API VulkanDevice;

class ENGINE_API VulkanRenderPass : public VulkanDisposable {
public:
    struct CreateInfo {
        std::vector<VkAttachmentDescription> attachments;
        std::vector<VkSubpassDescription> subPasses;
        std::vector<VkSubpassDependency> dependencies;
    };

    VulkanRenderPass() = default;
    explicit VulkanRenderPass(VulkanDevice& device, const CreateInfo& createInfo);
    ~VulkanRenderPass();
    VulkanRenderPass(const VulkanRenderPass& other) = delete;
    VulkanRenderPass(VulkanRenderPass&& other) noexcept;
    VulkanRenderPass& operator=(const VulkanRenderPass& other) = delete;
    VulkanRenderPass& operator=(VulkanRenderPass&& other) noexcept;
    void swap(VulkanRenderPass& other) noexcept;

    VkRenderPass& getHandle() {
        return renderPass;
    }

    const VkRenderPass& getHandle() const {
        return renderPass;
    }

    operator bool() const {
        return renderPass != VK_NULL_HANDLE;
    }

    void destroy() override;

private:
    VkDevice device{VK_NULL_HANDLE};
    VkRenderPass renderPass{VK_NULL_HANDLE};
};
} // namespace Engine
