#include "render_pass_outline.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

RenderPassOutline::RenderPassOutline(VulkanRenderer& vulkan, RenderResources& resources, AssetsManager& assetsManager,
                                     const Vector2i& viewport, const VulkanTexture& entityColor) :
    RenderPass{vulkan, viewport}, subpassOutline{vulkan, resources, assetsManager, entityColor} {

    // Color
    addAttachment({VK_FORMAT_R8G8B8A8_UNORM,
                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                   VK_IMAGE_ASPECT_COLOR_BIT},
                  VK_IMAGE_LAYOUT_UNDEFINED,
                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                  VK_ATTACHMENT_LOAD_OP_CLEAR);

    addSubpass(subpassOutline);
    init();
}

void RenderPassOutline::render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene) {
    VulkanRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.framebuffer = &getFbo();
    renderPassInfo.renderPass = &getRenderPass();
    renderPassInfo.offset = {0, 0};
    renderPassInfo.size = viewport;

    renderPassInfo.clearValues.resize(totalAttachments);
    renderPassInfo.clearValues[Attachments::Color].color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    vkb.beginRenderPass(renderPassInfo);
    vkb.setViewport({0, 0}, viewport);
    vkb.setScissor({0, 0}, viewport);

    subpassOutline.render(vkb, scene);

    vkb.endRenderPass();
}
