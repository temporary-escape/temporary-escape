#include "render_pass_non_hdr.hpp"
#include "render_pass_forward.hpp"

using namespace Engine;

RenderPassNonHdr::RenderPassNonHdr(VulkanRenderer& vulkan, Registry& registry, const Vector2i& viewport,
                                   const RenderPassForward& forward) :
    RenderPass{vulkan, viewport}, subpassNonHdr{vulkan, registry} {

    // Depth
    addAttachment(forward.getTexture(RenderPassForward::Depth), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ATTACHMENT_LOAD_OP_LOAD);
    // Forward
    addAttachment(forward.getTexture(RenderPassForward::Forward), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ATTACHMENT_LOAD_OP_LOAD);

    addSubpass(subpassNonHdr);
    init();
}

void RenderPassNonHdr::render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene) {
    VulkanRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.framebuffer = &getFbo();
    renderPassInfo.renderPass = &getRenderPass();
    renderPassInfo.offset = {0, 0};
    renderPassInfo.size = viewport;

    renderPassInfo.clearValues.resize(totalAttachments);
    renderPassInfo.clearValues[Attachments::Depth].depthStencil = {1.0f, 0};
    renderPassInfo.clearValues[Attachments::Forward].color = {{0.0f, 0.0f, 0.0f, 1.0f}};

    vkb.beginRenderPass(renderPassInfo);
    vkb.setViewport({0, 0}, viewport);
    vkb.setScissor({0, 0}, viewport);

    subpassNonHdr.render(vkb, scene);

    vkb.endRenderPass();
}
