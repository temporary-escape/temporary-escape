#include "render_pass_planet_surface.hpp"

using namespace Engine;

static auto logger = createLogger(__FILENAME__);

RenderPassPlanetSurface::RenderPassPlanetSurface(VulkanRenderer& vulkan, Registry& registry, const Vector2i& viewport) :
    RenderPass{vulkan, viewport},
    subpassPlanetHeightmap{vulkan, registry},
    subpassPlanetMoisture{vulkan, registry},
    subpassPlanetColor{vulkan, registry, *this} {

    logger.info("Creating render pass: {} viewport: {}", typeid(*this).name(), viewport);

    addAttachment({VK_FORMAT_R8_UNORM,
                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
                   VK_IMAGE_ASPECT_COLOR_BIT},
                  VK_IMAGE_LAYOUT_UNDEFINED,
                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                  VK_ATTACHMENT_LOAD_OP_DONT_CARE);

    addAttachment({VK_FORMAT_R8_UNORM,
                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
                   VK_IMAGE_ASPECT_COLOR_BIT},
                  VK_IMAGE_LAYOUT_UNDEFINED,
                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                  VK_ATTACHMENT_LOAD_OP_DONT_CARE);

    addAttachment({VK_FORMAT_R8G8B8A8_UNORM,
                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                   VK_IMAGE_ASPECT_COLOR_BIT},
                  VK_IMAGE_LAYOUT_UNDEFINED,
                  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                  VK_ATTACHMENT_LOAD_OP_DONT_CARE);

    addAttachment({VK_FORMAT_R8G8_UNORM,
                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                   VK_IMAGE_ASPECT_COLOR_BIT},
                  VK_IMAGE_LAYOUT_UNDEFINED,
                  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                  VK_ATTACHMENT_LOAD_OP_DONT_CARE);

    addSubpass(subpassPlanetHeightmap);
    addSubpass(subpassPlanetMoisture);
    addSubpass(subpassPlanetColor);
    init();
}

void RenderPassPlanetSurface::render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Rng& rng, const int index,
                                     const PlanetTypePtr& planetType) {
    VulkanRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.framebuffer = &getFbo();
    renderPassInfo.renderPass = &getRenderPass();
    renderPassInfo.offset = {0, 0};
    renderPassInfo.size = viewport;

    renderPassInfo.clearValues.resize(totalAttachments);
    renderPassInfo.clearValues[Attachments::Heightmap].color = {{1.0f, 0.0f, 0.0f, 1.0f}};
    renderPassInfo.clearValues[Attachments::Moisture].color = {{1.0f, 0.0f, 0.0f, 1.0f}};
    renderPassInfo.clearValues[Attachments::Color].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    renderPassInfo.clearValues[Attachments::MetallicRoughness].color = {{0.0f, 0.0f, 0.0f, 1.0f}};

    vkb.beginRenderPass(renderPassInfo);
    vkb.setViewport({0, 0}, viewport);
    vkb.setScissor({0, 0}, viewport);

    subpassPlanetHeightmap.render(vkb, rng, index, viewport.x);

    vkb.nextSubpass();
    subpassPlanetMoisture.render(vkb, rng, index, viewport.x);

    vkb.nextSubpass();
    subpassPlanetColor.render(vkb, planetType);

    vkb.endRenderPass();
}
