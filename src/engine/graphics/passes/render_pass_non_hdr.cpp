#include "render_pass_non_hdr.hpp"
#include "../../assets/assets_manager.hpp"
#include "../../scene/scene.hpp"
#include "../theme.hpp"

using namespace Engine;

RenderPassNonHDR::RenderPassNonHDR(const RenderOptions& options, VulkanRenderer& vulkan, RenderBufferPbr& buffer,
                                   RenderResources& resources, AssetsManager& assetsManager) :
    RenderPass{vulkan, buffer, "RenderPassNonHDR"},
    options{options},
    buffer{buffer},
    resources{resources},
    pipelineOutline{vulkan, assetsManager} {

    { // Forward
        AttachmentInfo attachment{};
        attachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;
        attachment.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        addAttachment(options.fxaa ? RenderBufferPbr::Attachment::Forward : RenderBufferPbr::Attachment::FXAA,
                      attachment);
    }

    addSubpass(
        {
            options.fxaa ? RenderBufferPbr::Attachment::Forward : RenderBufferPbr::Attachment::FXAA,
        },
        {});

    { // Dependency for Forward/FXAA
        DependencyInfo dependency{};
        dependency.srcStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dstAccessMask = VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                                   VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        addSubpassDependency(dependency);
    }

    addPipeline(pipelineOutline, 0);
}

void RenderPassNonHDR::beforeRender(VulkanCommandBuffer& vkb) {
    (void)vkb;
}

void RenderPassNonHDR::render(VulkanCommandBuffer& vkb, Scene& scene) {
    vkb.setViewport({0, 0}, getViewport());
    vkb.setScissor({0, 0}, getViewport());

    const auto selected = scene.getSelectedEntity();
    if (!selected) {
        return;
    }

    pipelineOutline.bind(vkb);

    const auto& texEntity = buffer.getAttachmentTexture(RenderBufferPbr::Attachment::Entity);
    const auto selectedColor = entityColor(selected->getHandle());
    const auto finalColor = Theme::primary * alpha(0.2);

    pipelineOutline.setColorFinal(finalColor);
    pipelineOutline.setColorSelected(selectedColor);
    pipelineOutline.setThickness(0.6f);
    pipelineOutline.flushConstants(vkb);
    pipelineOutline.setTextureEntity(texEntity);
    pipelineOutline.flushDescriptors(vkb);

    pipelineOutline.renderMesh(vkb, resources.getMeshFullScreenQuad());
}
