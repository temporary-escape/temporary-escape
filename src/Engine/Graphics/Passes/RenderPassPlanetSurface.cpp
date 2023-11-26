#include "RenderPassPlanetSurface.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Scene.hpp"

using namespace Engine;

RenderPassPlanetSurface::RenderPassPlanetSurface(VulkanRenderer& vulkan, RenderBufferPlanet& buffer,
                                                 RenderResources& resources, AssetsManager& assetsManager) :
    RenderPass{vulkan, buffer, "RenderPassPlanetSurface"},
    buffer{buffer},
    resources{resources},
    pipelinePlanetHeight{vulkan, assetsManager},
    pipelinePlanetMoisture{vulkan, assetsManager},
    pipelinePlanetColor{vulkan, assetsManager} {

    { // Heightmap
        AttachmentInfo attachment{};
        attachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        addAttachment(RenderBufferPlanet::Attachment::Heightmap, attachment);
    }

    { // Moisture
        AttachmentInfo attachment{};
        attachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        addAttachment(RenderBufferPlanet::Attachment::Moisture, attachment);
    }

    { // Color
        AttachmentInfo attachment{};
        attachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        addAttachment(RenderBufferPlanet::Attachment::Color, attachment);
    }

    { // Metallic Roughness
        AttachmentInfo attachment{};
        attachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        addAttachment(RenderBufferPlanet::Attachment::MetallicRoughness, attachment);
    }

    addSubpass(
        {
            RenderBufferPlanet::Attachment::Heightmap,
        },
        {});

    addPipeline(pipelinePlanetHeight, 0);

    addSubpass(
        {
            RenderBufferPlanet::Attachment::Moisture,
        },
        {});

    addPipeline(pipelinePlanetMoisture, 1);

    addSubpass(
        {
            RenderBufferPlanet::Attachment::Color,
            RenderBufferPlanet::Attachment::MetallicRoughness,
        },
        {
            RenderBufferPlanet::Attachment::Heightmap,
            RenderBufferPlanet::Attachment::Moisture,
        });

    addPipeline(pipelinePlanetColor, 2);
}

void RenderPassPlanetSurface::beforeRender(VulkanCommandBuffer& vkb) {
    (void)vkb;
}

void RenderPassPlanetSurface::render(VulkanCommandBuffer& vkb, Scene& scene) {
    (void)scene;

    vkb.setViewport({0, 0}, getViewport());
    vkb.setScissor({0, 0}, getViewport());

    renderHeight(vkb);

    vkb.nextSubpass();

    renderMoisture(vkb);

    vkb.nextSubpass();

    renderColor(vkb);
}

void RenderPassPlanetSurface::setSeed(const uint64_t value) {
    rng = std::mt19937_64{value};
}

void RenderPassPlanetSurface::setIndex(const int value) {
    index = value;
}

void RenderPassPlanetSurface::setPlanetType(PlanetTypePtr value) {
    planetType = std::move(value);
}

void RenderPassPlanetSurface::renderHeight(VulkanCommandBuffer& vkb) {
    pipelinePlanetHeight.bind(vkb);

    const float resMin = 0.01f;
    const float resMax = 5.0f;
    const float resolution = getViewport().x;
    const float seed = randomReal(rng, 0.0f, 1.0f) * 1000.0f;
    const float res1 = randomReal(rng, resMin, resMax);
    const float res2 = randomReal(rng, resMin, resMax);
    const float resMix = randomReal(rng, resMin, resMax);
    const float mixScale = randomReal(rng, 0.5f, 1.0f);
    const float doesRidged = std::floor(randomReal(rng, 0.0f, 4.0f));

    pipelinePlanetHeight.setIndex(index);
    pipelinePlanetHeight.setSeed(seed);
    pipelinePlanetHeight.setResolution(resolution);
    pipelinePlanetHeight.setRes1(res1);
    pipelinePlanetHeight.setRes2(res2);
    pipelinePlanetHeight.setResMix(resMix);
    pipelinePlanetHeight.setMixScale(mixScale);
    pipelinePlanetHeight.setDoesRidged(doesRidged);
    pipelinePlanetHeight.flushConstants(vkb);

    pipelinePlanetHeight.renderMesh(vkb, resources.getMeshFullScreenQuad());
}

void RenderPassPlanetSurface::renderMoisture(VulkanCommandBuffer& vkb) {
    pipelinePlanetMoisture.bind(vkb);

    const float resMin = 0.01f;
    const float resMax = 5.0f;
    const float resolution = getViewport().x;
    const float seed = randomReal(rng, 0.0f, 1.0f) * 1000.0f;
    const float res1 = randomReal(rng, resMin, resMax);
    const float res2 = randomReal(rng, resMin, resMax);
    const float resMix = randomReal(rng, resMin, resMax);
    const float mixScale = randomReal(rng, 0.5f, 1.0f);
    const float doesRidged = std::floor(randomReal(rng, 0.0f, 4.0f));

    pipelinePlanetMoisture.setIndex(index);
    pipelinePlanetMoisture.setSeed(seed);
    pipelinePlanetMoisture.setResolution(resolution);
    pipelinePlanetMoisture.setRes1(res1);
    pipelinePlanetMoisture.setRes2(res2);
    pipelinePlanetMoisture.setResMix(resMix);
    pipelinePlanetMoisture.setMixScale(mixScale);
    pipelinePlanetMoisture.setDoesRidged(doesRidged);
    pipelinePlanetMoisture.flushConstants(vkb);

    pipelinePlanetMoisture.renderMesh(vkb, resources.getMeshFullScreenQuad());
}

void RenderPassPlanetSurface::renderColor(VulkanCommandBuffer& vkb) {
    pipelinePlanetColor.bind(vkb);

    pipelinePlanetColor.setTextureBiome(planetType->getBiomeTexture()->getVulkanTexture());
    pipelinePlanetColor.setTextureRoughness(planetType->getRoughnessTexture()->getVulkanTexture());
    pipelinePlanetColor.setInputHeightmap(buffer.getAttachmentTexture(RenderBufferPlanet::Attachment::Heightmap));
    pipelinePlanetColor.setInputMoisture(buffer.getAttachmentTexture(RenderBufferPlanet::Attachment::Moisture));
    pipelinePlanetColor.flushDescriptors(vkb);

    pipelinePlanetColor.renderMesh(vkb, resources.getMeshFullScreenQuad());
}
