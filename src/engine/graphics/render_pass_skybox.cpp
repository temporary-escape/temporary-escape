#include "render_pass_skybox.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

RenderPassSkybox::RenderPassSkybox(VulkanRenderer& vulkan, AssetsManager& assetsManager, const Vector2i& viewport,
                                   const VulkanTexture& brdf) :
    RenderPass{vulkan, viewport}, subpassSkybox{vulkan, assetsManager, brdf} {

    logger.info("Creating render pass: {} viewport: {}", typeid(*this).name(), viewport);

    // Depth
    addAttachment({findDepthFormat(), VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT},
                  VK_IMAGE_LAYOUT_UNDEFINED,
                  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    // Forward
    addAttachment({VK_FORMAT_R16G16B16A16_SFLOAT,
                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                   VK_IMAGE_ASPECT_COLOR_BIT},
                  VK_IMAGE_LAYOUT_UNDEFINED,
                  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    addSubpass(subpassSkybox);
    init();
}

void RenderPassSkybox::render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene) {
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

    subpassSkybox.render(vkb, scene);

    vkb.endRenderPass();
}
