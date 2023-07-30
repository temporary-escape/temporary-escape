#include "render_pass_lighting.hpp"
#include "render_pass_opaque.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

RenderPassLighting::RenderPassLighting(VulkanRenderer& vulkan, RenderResources& resources, AssetsManager& assetsManager,
                                       const Vector2i& viewport, const RenderPassOpaque& opaque,
                                       const RenderPassSsao& ssao, const RenderPassShadows& shadows,
                                       const VulkanTexture& brdf, const VulkanTexture& forward) :
    RenderPass{vulkan, viewport}, subpassPbr{vulkan, resources, assetsManager, opaque, ssao, shadows, brdf} {

    // Forward
    addAttachment(forward,
                  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                  VK_ATTACHMENT_LOAD_OP_LOAD);

    addSubpass(subpassPbr);
    init();
}

void RenderPassLighting::render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene) {
    VulkanRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.framebuffer = &getFbo();
    renderPassInfo.renderPass = &getRenderPass();
    renderPassInfo.offset = {0, 0};
    renderPassInfo.size = viewport;

    renderPassInfo.clearValues.resize(totalAttachments);
    renderPassInfo.clearValues[Attachments::Forward].color = {{0.0f, 0.0f, 0.0f, 1.0f}};

    vkb.beginRenderPass(renderPassInfo);
    vkb.setViewport({0, 0}, viewport);
    vkb.setScissor({0, 0}, viewport);

    subpassPbr.render(vkb, scene);

    vkb.endRenderPass();
}
