#pragma once

#include "../utils/path.hpp"
#include "vg_types.hpp"

namespace Engine {
class VgDevice;

class VgRenderPass {
public:
    struct CreateInfo {
        std::vector<VkAttachmentDescription> attachments;
        std::vector<VkSubpassDescription> subPasses;
    };

    VgRenderPass() = default;
    explicit VgRenderPass(const Config& config, VgDevice& device, const CreateInfo& createInfo);
    ~VgRenderPass();
    VgRenderPass(const VgRenderPass& other) = delete;
    VgRenderPass(VgRenderPass&& other) noexcept;
    VgRenderPass& operator=(const VgRenderPass& other) = delete;
    VgRenderPass& operator=(VgRenderPass&& other) noexcept;
    void swap(VgRenderPass& other) noexcept;

    VkRenderPass& getHandle() {
        return renderPass;
    }

    const VkRenderPass& getHandle() const {
        return renderPass;
    }

    operator bool() const {
        return renderPass != VK_NULL_HANDLE;
    }

    void destroy();

private:
    VgDevice* device{nullptr};
    VkRenderPass renderPass{VK_NULL_HANDLE};
};
} // namespace Engine
