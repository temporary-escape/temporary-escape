#include "render_pass_fxaa.hpp"
#include "render_pass_opaque.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

RenderPassFxaa::RenderPassFxaa(VulkanRenderer& vulkan, RenderResources& resources, AssetsManager& assetsManager,
                               const Vector2i& viewport, const VulkanTexture& forward) :
    RenderPass{vulkan, viewport}, subpassFxaa{vulkan, resources, assetsManager, forward} {

    logger.info("Creating render pass: {} viewport: {}", typeid(*this).name(), viewport);

    // Forward
    addAttachment({VK_FORMAT_R16G16B16A16_SFLOAT,
                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                   VK_IMAGE_ASPECT_COLOR_BIT},
                  VK_IMAGE_LAYOUT_UNDEFINED,
                  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    addSubpass(subpassFxaa);
    init();
}

void RenderPassFxaa::render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene) {
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

    subpassFxaa.render(vkb, scene);

    vkb.endRenderPass();
}
