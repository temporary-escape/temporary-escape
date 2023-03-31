#pragma once

#include "render_subpass.hpp"

namespace Engine {
class ENGINE_API RenderPass : public NonCopyable {
public:
    struct AttachmentInfo {
        VkFormat format;
        VkImageUsageFlags usage;
        VkImageAspectFlags aspectMask;
    };

    explicit RenderPass(VulkanRenderer& vulkan, const Vector2i& viewport);
    virtual ~RenderPass() = default;
    NON_MOVEABLE(RenderPass);

    VulkanFramebuffer& getFbo() {
        return fbo;
    }

    const VulkanRenderPass& getRenderPass() const {
        return renderPass;
    }

    const VulkanTexture& getTexture(const uint32_t index) const {
        return *attachments[index];
    }

    const std::vector<VkAttachmentDescription>& getAttachmentDescriptions() const {
        return attachmentDescriptions;
    }

protected:
    VulkanRenderer& vulkan;
    Vector2i viewport;

    VkFormat findDepthFormat();
    void addAttachment(const AttachmentInfo& attachmentInfo, const VkImageLayout initialLayout,
                       VkImageLayout finalLayout, VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR);
    void addAttachment(const VulkanTexture& texture, const VkImageLayout initialLayout, VkImageLayout finalLayout,
                       VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_LOAD);
    void addSubpass(RenderSubpass& subpass) {
        subpasses.push_back(&subpass);
    }
    void init(bool compute = false);
    void transitionRead(VulkanCommandBuffer& vkb, const VulkanTexture& texture);
    void transitionWrite(VulkanCommandBuffer& vkb, const VulkanTexture& texture);

private:
    struct SubpassDescriptionData {
        std::vector<VkAttachmentReference> colorReferences;
        std::vector<VkAttachmentReference> inputsReferences;
        VkAttachmentReference depthReference{};
    };

    void createFbo();
    void createRenderPass();
    void addSubpass(const Span<uint32_t>& indexes, const Span<uint32_t>& inputs);

    VulkanFramebuffer fbo;
    VulkanRenderPass renderPass;
    std::vector<RenderSubpass*> subpasses;
    std::list<VulkanTexture> textures;
    std::vector<const VulkanTexture*> attachments;
    size_t depthIndex{UINT64_MAX};
    std::vector<VkImageView> attachmentViews;
    std::vector<VkAttachmentDescription> attachmentDescriptions;
    std::vector<VkSubpassDescription> subpassDescriptions;
    std::vector<VkSubpassDependency> dependencies;
    std::list<SubpassDescriptionData> subpassDescriptionData;
};
} // namespace Engine
