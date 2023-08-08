#pragma once

#include "../vulkan/vulkan_renderer.hpp"
#include "render_buffer.hpp"
#include "render_resources.hpp"

namespace Engine {
class ENGINE_API RenderPipeline;
class ENGINE_API Scene;

class ENGINE_API RenderPass {
public:
    struct AttachmentInfo {
        VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkImageLayout finalLayout;
        VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        VkClearValue clearColor;
    };

    struct DependencyInfo {
        VkPipelineStageFlags srcStageMask;
        VkPipelineStageFlags dstStageMask;
        VkAccessFlags srcAccessMask;
        VkAccessFlags dstAccessMask;
    };

    explicit RenderPass(VulkanRenderer& vulkan, RenderBuffer& renderBuffer, std::string name);
    virtual ~RenderPass() = default;
    NON_MOVEABLE(RenderPass);
    NON_COPYABLE(RenderPass);

    virtual void beforeRender(VulkanCommandBuffer& vkb);
    virtual void afterRender(VulkanCommandBuffer& vkb);
    virtual void render(VulkanCommandBuffer& vkb, Scene& scene) = 0;
    void create();
    void begin(VulkanCommandBuffer& vkb);
    void end(VulkanCommandBuffer& vkb);
    const std::string& getName() const {
        return name;
    }
    const VulkanFramebuffer& getFbo() const {
        return fbo;
    }
    const VulkanRenderPass& getRenderPass() const {
        return renderPass;
    }
    Vector2i getViewport() const {
        return {viewport.width, viewport.height};
    }
    void setExcluded(const bool value) {
        excluded = value;
    }
    bool isExcluded() const {
        return excluded;
    }

protected:
    void addPipeline(RenderPipeline& pipeline, uint32_t subpass);
    void addAttachment(uint32_t attachment, const AttachmentInfo& info);
    void addSubpass(const std::vector<uint32_t>& attachments, const std::vector<uint32_t>& inputs);
    void addSubpassDependency(const DependencyInfo& dependency);

private:
    struct SubpassDescriptionData {
        std::vector<VkAttachmentReference> colorReferences;
        std::vector<VkAttachmentReference> inputsReferences;
        VkAttachmentReference depthReference{};
        std::vector<uint32_t> attachments;
        std::vector<uint32_t> inputs;
    };

    void createRenderPass();
    void createFbo();
    uint32_t getAttachmentIndex(uint32_t attachment) const;

    VulkanRenderer& vulkan;
    RenderBuffer& renderBuffer;
    const std::string name;
    VulkanFramebuffer fbo;
    VulkanRenderPass renderPass;
    VulkanRenderPassBeginInfo renderPassBeginInfo;
    std::vector<std::tuple<RenderPipeline*, uint32_t>> pipelines;
    VkExtent2D viewport{0, 0};
    bool excluded{false};

    // Used only during creation
    std::vector<VkImageView> attachmentViews;
    std::vector<uint32_t> attachmentIndexes;
    std::vector<VkAttachmentDescription> attachmentDescriptions;
    std::vector<VkSubpassDescription> subpassDescriptions;
    std::list<SubpassDescriptionData> subpassDescriptionData;
    std::vector<VkSubpassDependency> dependencies;
    uint32_t depthAttachment{UINT32_MAX};
};
} // namespace Engine
