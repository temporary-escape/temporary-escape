#pragma once

#include "../utils/path.hpp"
#include "vg_types.hpp"

namespace Engine {
class VgRenderPass {
public:
    struct CreateInfo {
        std::vector<VkAttachmentDescription> attachments;
        std::vector<VkSubpassDescription> subPasses;
    };

    VgRenderPass() = default;
    explicit VgRenderPass(const Config& config, VkDevice device, const CreateInfo& createInfo);
    ~VgRenderPass();
    VgRenderPass(const VgRenderPass& other) = delete;
    VgRenderPass(VgRenderPass&& other) noexcept;
    VgRenderPass& operator=(const VgRenderPass& other) = delete;
    VgRenderPass& operator=(VgRenderPass&& other) noexcept;
    void swap(VgRenderPass& other) noexcept;

    VkRenderPass getHandle() const {
        return renderPass;
    }

    operator bool() const {
        return renderPass != VK_NULL_HANDLE;
    }

private:
    void cleanup();

    VkDevice device{VK_NULL_HANDLE};
    VkRenderPass renderPass{VK_NULL_HANDLE};
};
} // namespace Engine
