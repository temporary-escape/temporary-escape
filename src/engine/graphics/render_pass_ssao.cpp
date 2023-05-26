#include "render_pass_ssao.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

RenderPassSsao::RenderPassSsao(VulkanRenderer& vulkan, AssetsManager& assetsManager, const Vector2i& viewport,
                               const RenderPassOpaque& previous) :
    RenderPass{vulkan, viewport}, subpassSsao{vulkan, assetsManager, previous} {

    logger.info("Creating render pass: {} viewport: {}", typeid(*this).name(), viewport);

    // Ssao
    addAttachment({VK_FORMAT_R8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_ASPECT_COLOR_BIT},
                  VK_IMAGE_LAYOUT_UNDEFINED,
                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    addSubpass(subpassSsao);
    init();
}

void RenderPassSsao::render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene) {
    VulkanRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.framebuffer = &getFbo();
    renderPassInfo.renderPass = &getRenderPass();
    renderPassInfo.offset = {0, 0};
    renderPassInfo.size = viewport;

    renderPassInfo.clearValues.resize(totalAttachments);
    renderPassInfo.clearValues[Attachments::Ssao].color = {{1.0f, 1.0f, 1.0f, 1.0f}};

    vkb.beginRenderPass(renderPassInfo);
    vkb.setViewport({0, 0}, viewport);
    vkb.setScissor({0, 0}, viewport);

    subpassSsao.render(vkb, scene);

    vkb.endRenderPass();
}
