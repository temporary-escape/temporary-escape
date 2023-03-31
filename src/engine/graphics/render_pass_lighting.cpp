#include "render_pass_lighting.hpp"
#include "render_pass_opaque.hpp"

using namespace Engine;

RenderPassLighting::RenderPassLighting(VulkanRenderer& vulkan, Registry& registry, const Vector2i& viewport,
                                       const RenderPassOpaque& opaque, const RenderPassSsao& ssao,
                                       const VulkanTexture& brdf) :
    RenderPass{vulkan, viewport}, subpassPbr{vulkan, registry, opaque, ssao, brdf}, subpassSkybox{vulkan, registry} {

    // Forward
    addAttachment({VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_ASPECT_COLOR_BIT},
                  VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    addSubpass(subpassSkybox);
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

    subpassSkybox.render(vkb, scene);

    vkb.nextSubpass();
    subpassPbr.render(vkb, scene);

    vkb.endRenderPass();
}