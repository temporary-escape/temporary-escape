#include "render_pass_planet_normal.hpp"
#include "../../assets/assets_manager.hpp"
#include "../../scene/scene.hpp"

using namespace Engine;

RenderPassPlanetNormal::RenderPassPlanetNormal(VulkanRenderer& vulkan, RenderBufferPlanet& buffer,
                                               RenderResources& resources, AssetsManager& assetsManager) :
    RenderPass{vulkan, buffer, "RenderPassPlanetNormal"},
    buffer{buffer},
    resources{resources},
    pipelinePlanetNormal{vulkan, assetsManager} {

    { // Normal
        AttachmentInfo attachment{};
        attachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        addAttachment(RenderBufferPlanet::Attachment::Normal, attachment);
    }

    addSubpass(
        {
            RenderBufferPlanet::Attachment::Normal,
        },
        {});

    addPipeline(pipelinePlanetNormal, 0);
}

void RenderPassPlanetNormal::beforeRender(VulkanCommandBuffer& vkb) {
    buffer.transitionLayout(vkb,
                            RenderBufferPlanet::Attachment::Heightmap,
                            VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                            VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT,
                            VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
}

void RenderPassPlanetNormal::render(VulkanCommandBuffer& vkb, Scene& scene) {
    (void)scene;

    vkb.setViewport({0, 0}, getViewport());
    vkb.setScissor({0, 0}, getViewport());

    renderNormal(vkb);
}

void RenderPassPlanetNormal::setPlanetType(PlanetTypePtr value) {
    planetType = std::move(value);
}

void RenderPassPlanetNormal::renderNormal(VulkanCommandBuffer& vkb) {
    pipelinePlanetNormal.bind(vkb);

    const float waterLevel = planetType->getAtmosphere().waterLevel;
    const float resolution = getViewport().x;
    const float strength = glm::clamp(map(resolution, 128.0f, 2048.0f, 0.2f, 0.8f), 0.1f, 1.0f);

    pipelinePlanetNormal.setWaterLevel(waterLevel);
    pipelinePlanetNormal.setResolution(resolution);
    pipelinePlanetNormal.setStrength(strength);
    pipelinePlanetNormal.flushConstants(vkb);

    pipelinePlanetNormal.setTextureHeight(buffer.getAttachmentTexture(RenderBufferPlanet::Attachment::Heightmap));
    pipelinePlanetNormal.flushDescriptors(vkb);

    pipelinePlanetNormal.renderMesh(vkb, resources.getMeshFullScreenQuad());
}
