#include "render_pass_skybox_prefilter.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

RenderPassSkyboxPrefilter::RenderPassSkyboxPrefilter(VulkanRenderer& vulkan, RenderResources& resources,
                                                     AssetsManager& assetsManager, const Vector2i& viewport) :
    RenderPass{vulkan, viewport* Vector2i{2, 1}}, subpassSkyboxPrefilter{vulkan, resources, assetsManager} {

    addAttachment({VK_FORMAT_R16G16B16A16_UNORM,
                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                   VK_IMAGE_ASPECT_COLOR_BIT},
                  VK_IMAGE_LAYOUT_UNDEFINED,
                  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                  VK_ATTACHMENT_LOAD_OP_LOAD);

    addSubpass(subpassSkyboxPrefilter);
    init();
}

void RenderPassSkyboxPrefilter::render(VulkanCommandBuffer& vkb, const Vector2i& viewport,
                                       const VulkanTexture& skyboxColor, const Matrix4& projection,
                                       const Matrix4& view) {
    VulkanRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.framebuffer = &getFbo();
    renderPassInfo.renderPass = &getRenderPass();
    renderPassInfo.offset = {0, 0};
    renderPassInfo.size = viewport* Vector2i{2, 1};

    renderPassInfo.clearValues.resize(totalAttachments);
    renderPassInfo.clearValues[Attachments::Prefilter].color = {{0.0f, 0.0f, 0.0f, 1.0f}};

    subpassSkyboxPrefilter.reset();

    vkb.beginRenderPass(renderPassInfo);

    for (uint32_t i = 0; i < getMipMapLevels(viewport); i++) {
        const auto offset = mipMapOffset(viewport, i);
        const auto size = mipMapSize(viewport, i);

        vkb.setViewport(offset, size);
        vkb.setScissor(offset, size);

        const auto roughness = static_cast<float>(i) / static_cast<float>((getMipMapLevels(viewport) - 1));
        subpassSkyboxPrefilter.render(vkb, skyboxColor, projection, view, roughness);
    }

    vkb.endRenderPass();
}
