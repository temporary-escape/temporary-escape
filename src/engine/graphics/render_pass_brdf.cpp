#include "render_pass_brdf.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

RenderPassBrdf::RenderPassBrdf(VulkanRenderer& vulkan, RenderResources& resources, AssetsManager& assetsManager,
                               const Vector2i& viewport) :
    RenderPass{vulkan, viewport}, subpassBrdf{vulkan, resources, assetsManager} {

    logger.info("Creating render pass: {} viewport: {}", typeid(*this).name(), viewport);

    addAttachment({VK_FORMAT_R16G16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_ASPECT_COLOR_BIT},
                  VK_IMAGE_LAYOUT_UNDEFINED,
                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    addSubpass(subpassBrdf);
    init();
}

void RenderPassBrdf::render(VulkanCommandBuffer& vkb, const Vector2i& viewport) {
    VulkanRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.framebuffer = &getFbo();
    renderPassInfo.renderPass = &getRenderPass();
    renderPassInfo.offset = {0, 0};
    renderPassInfo.size = viewport;

    renderPassInfo.clearValues.resize(totalAttachments);
    renderPassInfo.clearValues[Attachments::Color].color = {{0.0f, 0.0f, 0.0f, 1.0f}};

    vkb.beginRenderPass(renderPassInfo);
    vkb.setViewport({0, 0}, viewport);
    vkb.setScissor({0, 0}, viewport);

    subpassBrdf.render(vkb);

    vkb.endRenderPass();
}
