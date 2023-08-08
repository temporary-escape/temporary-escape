#include "renderer_planet_surface.hpp"
#include "../scene/scene.hpp"
#include "../utils/exceptions.hpp"
#include "passes/render_pass_planet_normal.hpp"
#include "passes/render_pass_planet_surface.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

RendererPlanetSurface::RendererPlanetSurface(const Config& config, const Vector2i& viewport, VulkanRenderer& vulkan,
                                             RenderResources& resources, AssetsManager& assetsManager,
                                             VoxelShapeCache& voxelShapeCache) :
    RendererWork{config, vulkan, voxelShapeCache},
    vulkan{vulkan},
    viewport{viewport},
    renderBufferPlanet{viewport, vulkan} {

    try {
        addRenderPass(std::make_unique<RenderPassPlanetSurface>(vulkan, renderBufferPlanet, resources, assetsManager));
        addRenderPass(std::make_unique<RenderPassPlanetNormal>(vulkan, renderBufferPlanet, resources, assetsManager));
    } catch (...) {
        EXCEPTION_NESTED("Failed to setup render passes");
    }
    create();
}

void RendererPlanetSurface::update(Scene& scene) {
    if (isBusy()) {
        return;
    }

    auto planets = scene.getView<ComponentTransform, ComponentPlanet>();
    for (auto&& [handle, transform, planet] : planets.each()) {
        if (!planet.isGenerated() && planet.isHighRes()) {
            logger.info("Starting rendering planet: {} seed: {}", planet.getPlanetType()->getName(), planet.getSeed());

            work.entity = scene.fromHandle(handle);
            work.seed = planet.getSeed();
            work.planetType = planet.getPlanetType();
            startJobs(6);
            break;
        }
    }
}

void RendererPlanetSurface::finished() {
    logger.info("Finished rendering planet: {} seed: {}", work.planetType->getName(), work.seed);

    work.entity.getComponent<ComponentPlanet>().setTextures(vulkan, std::move(planetTextures));
    planetTextures = PlanetTextures{};
}

void RendererPlanetSurface::beforeRender(VulkanCommandBuffer& vkb, Scene& scene, const size_t job) {
    if (job == 0) {
        prepareCubemap(vkb);
    }

    auto camera = scene.getPrimaryCamera();
    if (!camera) {
        EXCEPTION("Scene has no camera");
    }
    camera->setViewport(viewport);

    auto& passPlanetSurface = getRenderPass<RenderPassPlanetSurface>();
    passPlanetSurface.setSeed(work.seed);
    passPlanetSurface.setIndex(static_cast<int>(job));
    passPlanetSurface.setPlanetType(work.planetType);

    auto& passPlanetNormal = getRenderPass<RenderPassPlanetNormal>();
    passPlanetNormal.setPlanetType(work.planetType);
}

void RendererPlanetSurface::postRender(VulkanCommandBuffer& vkb, Scene& scene, const size_t job) {
    copyTexture(vkb, RenderBufferPlanet::Attachment::Color, planetTextures.getColor(), job);
    copyTexture(vkb, RenderBufferPlanet::Attachment::MetallicRoughness, planetTextures.getMetallicRoughness(), job);
    copyTexture(vkb, RenderBufferPlanet::Attachment::Normal, planetTextures.getNormal(), job);

    if (job == 5) {
        vkb.generateMipMaps(planetTextures.getColor());
        vkb.generateMipMaps(planetTextures.getMetallicRoughness());
        vkb.generateMipMaps(planetTextures.getNormal());
    }
}

void RendererPlanetSurface::copyTexture(VulkanCommandBuffer& vkb, const uint32_t attachment,
                                        const VulkanTexture& target, int side) {
    /*renderBufferPlanet.transitionLayout(vkb,
                                        attachment,
                                        VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                        VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT,
                                        VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT);*/

    const auto& source = renderBufferPlanet.getAttachmentTexture(attachment);
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = source.getHandle();
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = source.getMipMaps();
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = source.getLayerCount();
    barrier.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcAccessMask =
        VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT;

    vkb.pipelineBarrier(VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
                        barrier);

    VkOffset3D offset = {
        static_cast<int32_t>(source.getExtent().width),
        static_cast<int32_t>(source.getExtent().height),
        1,
    };

    std::array<VkImageBlit, 1> blit{};
    blit[0].srcOffsets[0] = {0, 0, 0};
    blit[0].srcOffsets[1] = offset;
    blit[0].srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit[0].srcSubresource.mipLevel = 0;
    blit[0].srcSubresource.baseArrayLayer = 0;
    blit[0].srcSubresource.layerCount = 1;
    blit[0].dstOffsets[0] = {0, 0, 0};
    blit[0].dstOffsets[1] = offset;
    blit[0].dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit[0].dstSubresource.mipLevel = 0;
    blit[0].dstSubresource.baseArrayLayer = side;
    blit[0].dstSubresource.layerCount = 1;

    vkb.blitImage(source,
                  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                  target,
                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                  blit,
                  VK_FILTER_NEAREST);
}

void RendererPlanetSurface::prepareTexture(VulkanCommandBuffer& vkb, const VulkanTexture& target) {
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = target.getHandle();
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = target.getMipMaps();
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = target.getLayerCount();
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkb.pipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, barrier);
}

void RendererPlanetSurface::prepareCubemap(VulkanCommandBuffer& vkb) {
    planetTextures.dispose(vulkan);
    planetTextures = PlanetTextures{};

    const auto extent = renderBufferPlanet.getAttachmentTexture(RenderBufferPlanet::Attachment::Color).getExtent();
    const Vector2i textureSize{extent.width, extent.height};

    VulkanTexture::CreateInfo textureInfo{};
    textureInfo.image.format = VK_FORMAT_R8G8B8A8_UNORM;
    textureInfo.image.imageType = VK_IMAGE_TYPE_2D;
    textureInfo.image.extent = {static_cast<uint32_t>(textureSize.x), static_cast<uint32_t>(textureSize.y), 1};
    textureInfo.image.mipLevels = getMipMapLevels(textureSize);
    textureInfo.image.arrayLayers = 6;
    textureInfo.image.tiling = VK_IMAGE_TILING_OPTIMAL;
    textureInfo.image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    textureInfo.image.usage =
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    textureInfo.image.samples = VK_SAMPLE_COUNT_1_BIT;
    textureInfo.image.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    textureInfo.image.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    textureInfo.view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    textureInfo.view.format = VK_FORMAT_R8G8B8A8_UNORM;
    textureInfo.view.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    textureInfo.view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    textureInfo.view.subresourceRange.baseMipLevel = 0;
    textureInfo.view.subresourceRange.levelCount = getMipMapLevels(textureSize);
    textureInfo.view.subresourceRange.baseArrayLayer = 0;
    textureInfo.view.subresourceRange.layerCount = 6;

    textureInfo.sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    textureInfo.sampler.magFilter = VK_FILTER_LINEAR;
    textureInfo.sampler.minFilter = VK_FILTER_LINEAR;
    textureInfo.sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.anisotropyEnable = VK_FALSE;
    textureInfo.sampler.maxAnisotropy = 1.0f;
    textureInfo.sampler.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    textureInfo.sampler.unnormalizedCoordinates = VK_FALSE;
    textureInfo.sampler.compareEnable = VK_FALSE;
    textureInfo.sampler.compareOp = VK_COMPARE_OP_ALWAYS;
    textureInfo.sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    textureInfo.sampler.minLod = 0.0f;
    textureInfo.sampler.maxLod = static_cast<float>(textureInfo.image.mipLevels);

    planetTextures.getColor() = vulkan.createTexture(textureInfo);

    textureInfo.image.format = VK_FORMAT_R8G8_UNORM;
    textureInfo.view.format = VK_FORMAT_R8G8_UNORM;

    planetTextures.getMetallicRoughness() = vulkan.createTexture(textureInfo);

    textureInfo.image.format = VK_FORMAT_R8G8B8A8_UNORM;
    textureInfo.view.format = VK_FORMAT_R8G8B8A8_UNORM;

    planetTextures.getNormal() = vulkan.createTexture(textureInfo);

    prepareTexture(vkb, planetTextures.getColor());
    prepareTexture(vkb, planetTextures.getMetallicRoughness());
    prepareTexture(vkb, planetTextures.getNormal());
}
