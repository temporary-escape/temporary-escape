#include "render_pass_forward.hpp"
#include "render_pass_lighting.hpp"
#include "render_pass_opaque.hpp"

using namespace Engine;

static auto logger = createLogger(__FILENAME__);

RenderPassForward::RenderPassForward(VulkanRenderer& vulkan, Registry& registry, const Vector2i& viewport,
                                     const RenderPassOpaque& opaque, const RenderPassLighting& lighting) :
    RenderPass{vulkan, viewport}, subpassForward{vulkan, registry} {

    logger.info("Creating render pass: {} viewport: {}", typeid(*this).name(), viewport);

    // Depth
    addAttachment(opaque.getTexture(RenderPassOpaque::Depth),
                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                  VK_ATTACHMENT_LOAD_OP_LOAD,
                  VK_ATTACHMENT_LOAD_OP_LOAD);
    // Forward
    addAttachment(lighting.getTexture(RenderPassLighting::Forward),
                  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                  VK_ATTACHMENT_LOAD_OP_LOAD);

    addSubpass(subpassForward);
    init();
}

void RenderPassForward::render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene) {
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

    subpassForward.render(vkb, scene);

    vkb.endRenderPass();
}
