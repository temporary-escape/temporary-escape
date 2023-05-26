#include "render_pass_planet_normal.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

RenderPassPlanetNormal::RenderPassPlanetNormal(VulkanRenderer& vulkan, AssetsManager& assetsManager,
                                               const Vector2i& viewport) :
    RenderPass{vulkan, viewport}, subpassPlanetNormal{vulkan, assetsManager} {

    logger.info("Creating render pass: {} viewport: {}", typeid(*this).name(), viewport);

    addAttachment({VK_FORMAT_R8G8B8A8_UNORM,
                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                   VK_IMAGE_ASPECT_COLOR_BIT},
                  VK_IMAGE_LAYOUT_UNDEFINED,
                  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                  VK_ATTACHMENT_LOAD_OP_DONT_CARE);

    addSubpass(subpassPlanetNormal);
    init();
}

void RenderPassPlanetNormal::render(VulkanCommandBuffer& vkb, const Vector2i& viewport,
                                    const VulkanTexture& heightmapTexture, const PlanetTypePtr& planetType) {
    VulkanRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.framebuffer = &getFbo();
    renderPassInfo.renderPass = &getRenderPass();
    renderPassInfo.offset = {0, 0};
    renderPassInfo.size = viewport;

    renderPassInfo.clearValues.resize(totalAttachments);
    renderPassInfo.clearValues[Attachments::Normal].color = {{1.0f, 0.0f, 0.0f, 1.0f}};

    vkb.beginRenderPass(renderPassInfo);
    vkb.setViewport({0, 0}, viewport);
    vkb.setScissor({0, 0}, viewport);

    subpassPlanetNormal.render(vkb, heightmapTexture, viewport.x, planetType);

    vkb.endRenderPass();
}
