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
    const auto& texture = buffer.getAttachmentTexture(RenderBufferPlanet::Attachment::Heightmap);
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = texture.getHandle();
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = texture.getMipMaps();
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = texture.getLayerCount();
    barrier.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask =
        VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;

    vkb.pipelineBarrier(VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                        barrier);
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
