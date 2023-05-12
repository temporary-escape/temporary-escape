#include "render_pass_skybox_irradiance.hpp"

using namespace Engine;

static auto logger = createLogger(__FILENAME__);

RenderPassSkyboxIrradiance::RenderPassSkyboxIrradiance(VulkanRenderer& vulkan, Registry& registry,
                                                       const Vector2i& viewport) :
    RenderPass{vulkan, viewport* Vector2i{2, 1}}, subpassSkyboxIrradiance{vulkan, registry} {

    logger.info("Creating render pass: {} viewport: {}", typeid(*this).name(), viewport);

    addAttachment({VK_FORMAT_R16G16B16A16_UNORM,
                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                   VK_IMAGE_ASPECT_COLOR_BIT},
                  VK_IMAGE_LAYOUT_UNDEFINED,
                  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                  VK_ATTACHMENT_LOAD_OP_LOAD);

    addSubpass(subpassSkyboxIrradiance);
    init();
}

void RenderPassSkyboxIrradiance::render(VulkanCommandBuffer& vkb, const Vector2i& viewport,
                                        const VulkanTexture& skyboxColor, const Matrix4& projection,
                                        const Matrix4& view) {
    VulkanRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.framebuffer = &getFbo();
    renderPassInfo.renderPass = &getRenderPass();
    renderPassInfo.offset = {0, 0};
    renderPassInfo.size = viewport* Vector2i{2, 1};

    renderPassInfo.clearValues.resize(totalAttachments);
    renderPassInfo.clearValues[Attachments::Irradiance].color = {{0.0f, 1.0f, 0.0f, 1.0f}};

    subpassSkyboxIrradiance.reset();

    for (uint32_t i = 0; i < getMipMapLevels(viewport); i++) {
        renderPassInfo.offset = mipMapOffset(viewport, i);
        renderPassInfo.size = mipMapSize(viewport, i);

        vkb.beginRenderPass(renderPassInfo);
        vkb.setViewport(renderPassInfo.offset, renderPassInfo.size);
        vkb.setScissor(renderPassInfo.offset, renderPassInfo.size);

        subpassSkyboxIrradiance.render(vkb, skyboxColor, projection, view);

        vkb.endRenderPass();
    }
}
