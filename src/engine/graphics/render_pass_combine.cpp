#include "render_pass_combine.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

RenderPassCombine::RenderPassCombine(const Config& config, VulkanRenderer& vulkan, RenderResources& resources,
                                     AssetsManager& assetsManager, const Vector2i& viewport, const VulkanTexture& dst,
                                     const VulkanTexture& color, const VulkanTexture& blured) :
    RenderPass{vulkan, viewport}, subpassCombine{config, vulkan, resources, assetsManager, color, blured} {

    logger.info("Creating render pass: {} viewport: {}", typeid(*this).name(), viewport);

    addAttachment(
        dst, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_DONT_CARE);

    addSubpass(subpassCombine);
    init();
}

void RenderPassCombine::render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene) {
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

    subpassCombine.render(vkb, scene);

    vkb.endRenderPass();
}
