#include "planet_generator.hpp"

using namespace Engine;

static auto logger = createLogger(__FILENAME__);

static void copyTexture(VulkanCommandBuffer& vkb, const VulkanTexture& source, const VulkanTexture& target, int side) {
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

static void transitionTextureDst(VulkanCommandBuffer& vkb, const VulkanTexture& texture) {
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = texture.getHandle();
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = texture.getMipMaps();
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = texture.getLayerCount();
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkb.pipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, barrier);
}

static void transitionTextureShaderRead(VulkanCommandBuffer& vkb, const VulkanTexture& texture) {
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = texture.getHandle();
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = texture.getMipMaps();
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = texture.getLayerCount();
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkb.pipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, barrier);
}

PlanetGenerator::PlanetGenerator(const Config& config, VulkanRenderer& vulkan, Registry& registry) :
    config{config}, vulkan{vulkan} {

    // clang-format off
    renderPasses.planetSurface = std::make_unique<RenderPassPlanetSurface>(
        vulkan, registry, Vector2i{config.planetTextureSize, config.planetTextureSize});
    renderPasses.planetNormal = std::make_unique<RenderPassPlanetNormal>(
        vulkan, registry, Vector2i{config.planetTextureSize, config.planetTextureSize});
    // clang-format on
}

void PlanetGenerator::enqueue(const uint64_t seed, const PlanetTypePtr& planetType,
                              std::function<void(PlanetTextures)> callback) {
    if (work.seed || work.isRunning) {
        EXCEPTION("Can not enqueue planet texture generation, already in progress");
    }

    work.seed = seed;
    work.side = 0;
    work.planetType = planetType;
    work.callback = std::move(callback);
}

void PlanetGenerator::update(Scene& scene) {
    if (work.isRunning) {
        return;
    }

    auto planets = scene.getView<ComponentTransform, ComponentPlanet>();
    for (auto&& [id, transform, planet] : planets.each()) {
        if (!planet.isGenerated()) {
            const EntityWeakPtr entity = scene.getEntityById(static_cast<entt::id_type>(id));

            enqueue(planet.getSeed(), planet.getPlanetType(), [this, entity](PlanetTextures textures) {
                (void)textures;
                (void)entity;

                if (auto ptr = entity.lock(); ptr != nullptr) {
                    ptr->getComponent<ComponentPlanet>().setTextures(vulkan, std::move(textures));
                }
            });

            break;
        }
    }
}

void PlanetGenerator::run() {
    if (work.seed && !work.isRunning) {
        work.isRunning = true;
        fence = VulkanFence{vulkan};

        logger.info("Starting planet texture generation seed: {} side: {}", work.seed, work.side);
        startWork();

    } else if (work.isRunning && fence) {
        if (fence.isDone()) {

            work.side++;

            if (work.side >= 6) {
                logger.info("Done planet texture generation seed: {}", work.seed);
                work.isRunning = false;
                work.seed = 0;
                work.callback(std::move(planetTextures));
            } else {
                logger.info("Continuing planet texture generation seed: {} side: {}", work.seed, work.side);
                startWork();
            }
        }
    }
}

void PlanetGenerator::startWork() {
    fence.reset();

    vulkan.dispose(std::move(vkb));
    vkb = vulkan.createCommandBuffer();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkb.start(beginInfo);

    if (work.side == 0) {
        prepareCubemap();
    }

    Rng rng{work.seed};

    const auto viewport = Vector2i{config.planetTextureSize, config.planetTextureSize};
    renderPasses.planetSurface->render(vkb, viewport, rng, work.side, work.planetType);

    const auto& heightmap = renderPasses.planetSurface->getTexture(RenderPassPlanetSurface::Attachments::Heightmap);
    renderPasses.planetNormal->render(vkb, viewport, heightmap);

    const auto& color = renderPasses.planetSurface->getTexture(RenderPassPlanetSurface::Attachments::Color);
    const auto& metallicRoughness =
        renderPasses.planetSurface->getTexture(RenderPassPlanetSurface::Attachments::MetallicRoughness);
    const auto& normal = renderPasses.planetNormal->getTexture(RenderPassPlanetNormal::Attachments::Normal);

    copyTexture(vkb, color, planetTextures.getColor(), work.side);
    copyTexture(vkb, metallicRoughness, planetTextures.getMetallicRoughness(), work.side);
    copyTexture(vkb, normal, planetTextures.getNormal(), work.side);

    if (work.side == 5) {
        transitionTextureShaderRead(vkb, planetTextures.getColor());
        transitionTextureShaderRead(vkb, planetTextures.getMetallicRoughness());
        transitionTextureShaderRead(vkb, planetTextures.getNormal());
    }

    vkb.end();
    vulkan.submitCommandBuffer(vkb, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, {}, {}, fence);
}

void PlanetGenerator::prepareCubemap() {
    planetTextures.dispose(vulkan);
    planetTextures = PlanetTextures{};

    auto size = Vector2i{config.planetTextureSize, config.planetTextureSize};

    logger.debug("Preparing planet cubemap of size: {}", size);

    VulkanTexture::CreateInfo textureInfo{};
    textureInfo.image.format = VK_FORMAT_R8G8B8A8_UNORM;
    textureInfo.image.imageType = VK_IMAGE_TYPE_2D;
    textureInfo.image.extent = {static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y), 1};
    textureInfo.image.mipLevels = getMipMapLevels(size);
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
    textureInfo.view.subresourceRange.levelCount = 1;
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

    transitionTextureDst(vkb, planetTextures.getColor());
    transitionTextureDst(vkb, planetTextures.getMetallicRoughness());
    transitionTextureDst(vkb, planetTextures.getNormal());
}
