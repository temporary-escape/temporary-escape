#include "skybox_generator.hpp"
#include "../assets/assets_manager.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

static const float PI = static_cast<float>(std::atan(1) * 4);

// The projection matrix that will be used to generate the skybox.
// This must be 90 degrees view (PI/2)
static const glm::mat4 captureProjectionMatrix = glm::perspective(PI / 2.0, 1.0, 0.1, 1000.0);

static const std::array<Vector3, 6> captureViewCenter = {
    Vector3{-1.0f, 0.0f, 0.0f},
    Vector3{1.0f, 0.0f, 0.0f},
    Vector3{0.0f, 1.0f, 0.0f},
    Vector3{0.0f, -1.0f, 0.0f},
    Vector3{0.0f, 0.0f, 1.0f},
    Vector3{0.0f, 0.0f, -1.0f},
};

static const std::array<Vector3, 6> captureViewUp = {
    Vector3{0.0f, -1.0f, 0.0f},
    Vector3{0.0f, -1.0f, 0.0f},
    Vector3{0.0f, 0.0f, 1.0f},
    Vector3{0.0f, 0.0f, -1.0f},
    Vector3{0.0f, -1.0f, 0.0f},
    Vector3{0.0f, -1.0f, 0.0f},
};

static void copyTexture(VulkanCommandBuffer& vkb, const VulkanTexture& source, const VulkanTexture& target,
                        const int side) {
    VkOffset3D extend = {
        static_cast<int32_t>(source.getExtent().width),
        static_cast<int32_t>(source.getExtent().height),
        1,
    };

    std::array<VkImageBlit, 1> blit{};
    blit[0].srcOffsets[0] = {0, 0, 0};
    blit[0].srcOffsets[1] = extend;
    blit[0].srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit[0].srcSubresource.mipLevel = 0;
    blit[0].srcSubresource.baseArrayLayer = 0;
    blit[0].srcSubresource.layerCount = 1;
    blit[0].dstOffsets[0] = {0, 0, 0};
    blit[0].dstOffsets[1] = extend;
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

static void copyTexture2(VulkanCommandBuffer& vkb, const VulkanTexture& source, const VulkanTexture& target,
                         const Vector2i& size, const int side) {
    VkOffset3D extend = {
        size.x,
        size.y,
        1,
    };

    std::array<VkImageBlit, 1> blit{};
    blit[0].srcOffsets[0] = {0, 0, 0};
    blit[0].srcOffsets[1] = extend;
    blit[0].srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit[0].srcSubresource.mipLevel = 0;
    blit[0].srcSubresource.baseArrayLayer = 0;
    blit[0].srcSubresource.layerCount = 1;
    blit[0].dstOffsets[0] = {0, 0, 0};
    blit[0].dstOffsets[1] = extend;
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

static void copyMipMaps(VulkanCommandBuffer& vkb, const VulkanTexture& source, const VulkanTexture& target,
                        const Vector2i& size, const int side) {
    const auto levels = getMipMapLevels(size) - 1;

    for (auto i = 1; i <= levels; i++) {
        const auto levelSize = mipMapSize(size, i);
        const auto levelOffset = mipMapOffset(size, i);

        std::array<VkImageBlit, 1> blit{};
        blit[0].srcOffsets[0] = {
            levelOffset.x,
            levelOffset.y,
            0,
        };
        blit[0].srcOffsets[1] = {
            levelOffset.x + levelSize.x,
            levelOffset.y + levelSize.y,
            1,
        };
        blit[0].srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit[0].srcSubresource.mipLevel = 0;
        blit[0].srcSubresource.baseArrayLayer = 0;
        blit[0].srcSubresource.layerCount = 1;
        blit[0].dstOffsets[0] = {
            0,
            0,
            0,
        };
        blit[0].dstOffsets[1] = {
            levelSize.x,
            levelSize.y,
            1,
        };
        blit[0].dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit[0].dstSubresource.mipLevel = i;
        blit[0].dstSubresource.baseArrayLayer = side;
        blit[0].dstSubresource.layerCount = 1;

        vkb.blitImage(source,
                      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                      target,
                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                      blit,
                      VK_FILTER_NEAREST);
    }
}

static void transitionTextureSrc(VulkanCommandBuffer& vkb, const VulkanTexture& texture) {
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = texture.getHandle();
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = texture.getMipMaps();
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = texture.getLayerCount();
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkb.pipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, barrier);
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

SkyboxGenerator::SkyboxGenerator(const Config& config, VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    config{config}, vulkan{vulkan} {

    // clang-format off
    renderPasses.skyboxColor = std::make_unique<RenderPassSkyboxColor>(
        vulkan, assetsManager, Vector2i{config.graphics.skyboxSize, config.graphics.skyboxSize});
    renderPasses.skyboxIrradiance = std::make_unique<RenderPassSkyboxIrradiance>(
            vulkan, assetsManager, Vector2i{config.graphics.skyboxIrradianceSize, config.graphics.skyboxIrradianceSize});
    renderPasses.skyboxPrefilter = std::make_unique<RenderPassSkyboxPrefilter>(
            vulkan, assetsManager, Vector2i{config.graphics.skyboxPrefilterSize, config.graphics.skyboxPrefilterSize});
    // clang-format on

    textures.star = assetsManager.getTextures().find("system_map_sun");
}

bool SkyboxGenerator::isBusy() const {
    return work.seed || work.isRunning;
}

void SkyboxGenerator::enqueue(const uint64_t seed, std::function<void(SkyboxTextures)> callback) {
    if (isBusy()) {
        EXCEPTION("Can not enqueue skybox texture generation, already in progress");
    }

    logger.info("Adding skybox work seed: {}", seed);

    work.seed = seed;
    work.side = 0;
    work.postProcess = false;
    work.callback = std::move(callback);
}

void SkyboxGenerator::update(Scene& scene) {
    if (work.isRunning) {
        return;
    }

    auto skyboxes = scene.getView<ComponentSkybox>();
    for (auto&& [id, skybox] : skyboxes.each()) {
        if (!skybox.isGenerated()) {
            const EntityWeakPtr entity = scene.getEntityById(static_cast<entt::id_type>(id));

            enqueue(skybox.getSeed(), [this, entity](SkyboxTextures result) {
                if (auto ptr = entity.lock(); ptr != nullptr) {
                    ptr->getComponent<ComponentSkybox>().setTextures(vulkan, std::move(result));
                }
            });

            break;
        }
    }
}

void SkyboxGenerator::run() {
    if (work.seed && !work.isRunning) {
        work.isRunning = true;
        fence = VulkanFence{vulkan};

        logger.info("Starting skybox texture generation seed: {} side: {}", work.seed, work.side);
        startWork();

    } else if (work.isRunning && fence) {
        if (fence.isDone()) {

            work.side++;

            if (work.side >= 6 && work.postProcess) {
                logger.info("Done skybox texture generation seed: {}", work.seed);
                work.isRunning = false;
                work.seed = 0;
                work.side = 0;
                work.postProcess = false;
                work.callback(std::move(skyboxTextures));
            } else {
                if (work.side >= 6) {
                    work.side = 0;
                    work.postProcess = true;
                }

                logger.info("Continuing skybox texture generation seed: {} side: {} postprocess: {}",
                            work.seed,
                            work.side,
                            work.postProcess);
                startWork();
            }
        }
    }
}

void SkyboxGenerator::startWork() {
    fence.reset();

    vulkan.dispose(std::move(vkb));
    vkb = vulkan.createCommandBuffer();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkb.start(beginInfo);

    if (work.side == 0 && !work.postProcess) {
        prepareCubemap();
        prepareScene();
        prepareProperties();
    }

    auto camera = work.scene->getPrimaryCamera();
    if (!camera) {
        EXCEPTION("Scene has no camera");
    }
    camera->lookAt({0.0f, 0.0f, 0.0f}, captureViewCenter[work.side], captureViewUp[work.side]);
    camera->setProjection(90.0f);

    // First stage is to render the skybox color texture
    if (!work.postProcess) {
        // Render the skybox color first
        const auto viewport = Vector2i{config.graphics.skyboxSize, config.graphics.skyboxSize};
        renderPasses.skyboxColor->render(vkb, viewport, *work.scene);

        // Grab the framebuffer texture representing the color
        const auto& color = renderPasses.skyboxColor->getTexture(RenderPassSkyboxColor::Color);

        // Copy the color texture
        transitionTextureSrc(vkb, color);
        copyTexture(vkb, color, skyboxTextures.getTexture(), work.side);

        if (work.side == 5) {
            vkb.generateMipMaps(skyboxTextures.getTexture());
        }
    }
    // Second stage is to create prefilter and irradiance textures
    else {
        // Render the irradiance texture
        auto viewport = Vector2i{config.graphics.skyboxIrradianceSize, config.graphics.skyboxIrradianceSize};
        const auto view = glm::lookAt(Vector3{0, 0, 0}, captureViewCenter[work.side], captureViewUp[work.side]);
        renderPasses.skyboxIrradiance->render(
            vkb, viewport, skyboxTextures.getTexture(), captureProjectionMatrix, view);

        // Grab the framebuffer texture representing the irradiance
        const auto& irradiance = renderPasses.skyboxIrradiance->getTexture(RenderPassSkyboxIrradiance::Irradiance);

        // Copy the irradiance texture
        copyTexture2(vkb, irradiance, skyboxTextures.getIrradiance(), viewport, work.side);
        copyMipMaps(vkb, irradiance, skyboxTextures.getIrradiance(), viewport, work.side);

        // Render the prefilter texture
        viewport = Vector2i{config.graphics.skyboxPrefilterSize, config.graphics.skyboxPrefilterSize};
        renderPasses.skyboxPrefilter->render(vkb, viewport, skyboxTextures.getTexture(), captureProjectionMatrix, view);

        // Grab the framebuffer texture representing the irradiance
        const auto& prefilter = renderPasses.skyboxPrefilter->getTexture(RenderPassSkyboxPrefilter::Prefilter);

        // Copy the prefilter texture
        copyTexture2(vkb, prefilter, skyboxTextures.getPrefilter(), viewport, work.side);
        copyMipMaps(vkb, prefilter, skyboxTextures.getPrefilter(), viewport, work.side);

        if (work.side == 5) {
            // vkb.generateMipMaps(skyboxTextures.getIrradiance());
            // vkb.generateMipMaps(skyboxTextures.getPrefilter());
            transitionTextureShaderRead(vkb, skyboxTextures.getIrradiance());
            transitionTextureShaderRead(vkb, skyboxTextures.getPrefilter());
        }
    }

    vkb.end();
    vulkan.submitCommandBuffer(vkb, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, {}, {}, fence);
}

void SkyboxGenerator::prepareScene() {
    work.scene = std::make_unique<Scene>();

    auto entity = work.scene->createEntity();
    auto& transform = entity->addComponent<ComponentTransform>();
    entity->addComponent<ComponentCamera>(transform);

    work.scene->setPrimaryCamera(entity);
}

void SkyboxGenerator::prepareProperties() {
    Rng rng{work.seed};
    prepareStars(rng, {5.5f, 5.5f}, 50000);
    prepareStars(rng, {11.0f, 11.0f}, 1000);
    prepareNebulas(rng);
}

void SkyboxGenerator::prepareNebulas(SkyboxGenerator::Rng& rng) const {
    std::uniform_real_distribution<float> dist{0.0f, 1.0f};

    while (true) {
        const auto scale = dist(rng) * 0.5f + 0.25f;
        const auto intensity = dist(rng) * 0.2f + 0.9f;
        const auto color = Color4{dist(rng), dist(rng), dist(rng), 1.0f};
        const auto falloff = dist(rng) * 3.0f + 3.0f;
        const auto offset =
            Vector3{dist(rng) * 2000.0f - 1000.0f, dist(rng) * 2000.0f - 1000.0f, dist(rng) * 2000.0f - 1000.0f};

        auto entity = work.scene->createEntity();
        entity->addComponent<ComponentTransform>();
        auto& nebula = entity->addComponent<ComponentNebula>();

        nebula.setColor(color);
        nebula.setOffset({offset, 1.0f});
        nebula.setScale(scale);
        nebula.setIntensity(intensity * 0.8f);
        nebula.setFalloff(falloff);

        if (dist(rng) < 0.5f) {
            break;
        }
    }
}

void SkyboxGenerator::prepareStars(Rng& rng, const Vector2& size, const size_t count) {
    logger.debug("Preparing skybox vertex buffer for: {} stars", count);

    std::uniform_real_distribution<float> distUV{0.0f, 1.0f};
    std::uniform_real_distribution<float> distColor(0.9f, 1.0f);
    std::uniform_real_distribution<float> distBrightness(0.2f, 1.0f);

    auto entity = work.scene->createEntity();
    entity->addComponent<ComponentTransform>();
    auto& pointCloud = entity->addComponent<ComponentPointCloud>(textures.star);

    for (size_t i = 0; i < count; i++) {
        const auto u = distUV(rng);
        const auto v = distUV(rng);
        const auto theta = 2.0 * PI * u;
        const auto phi = acos(2.0 * v - 1.0);
        const auto x = sin(phi) * cos(theta);
        const auto y = sin(phi) * sin(theta);
        const auto z = cos(phi);
        const auto position = Vector3{
            static_cast<float>(x),
            static_cast<float>(y),
            static_cast<float>(z),
        };

        const auto color = Color4{distColor(rng), distColor(rng), distColor(rng), distBrightness(rng)};
        pointCloud.add(position * 100.0f, size * 100.0f, color);
    }
}

void SkyboxGenerator::prepareCubemap() {
    skyboxTextures.dispose(vulkan);
    skyboxTextures = SkyboxTextures{};

    auto size = Vector2i{config.graphics.skyboxSize, config.graphics.skyboxSize};

    logger.debug("Preparing skybox cubemap of size: {}", size);

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
    textureInfo.view.subresourceRange.levelCount = textureInfo.image.mipLevels;
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

    skyboxTextures.getTexture() = vulkan.createTexture(textureInfo);

    size = Vector2i{config.graphics.skyboxPrefilterSize, config.graphics.skyboxPrefilterSize};

    textureInfo.image.format = VK_FORMAT_R16G16B16A16_UNORM;
    textureInfo.view.format = VK_FORMAT_R16G16B16A16_UNORM;
    textureInfo.image.extent = {static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y), 1};
    textureInfo.image.mipLevels = getMipMapLevels(size);
    textureInfo.sampler.maxLod = static_cast<float>(textureInfo.image.mipLevels);
    textureInfo.view.subresourceRange.levelCount = textureInfo.image.mipLevels;

    skyboxTextures.getPrefilter() = vulkan.createTexture(textureInfo);

    size = Vector2i{config.graphics.skyboxIrradianceSize, config.graphics.skyboxIrradianceSize};

    textureInfo.image.format = VK_FORMAT_R16G16B16A16_UNORM;
    textureInfo.view.format = VK_FORMAT_R16G16B16A16_UNORM;
    textureInfo.image.extent = {static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y), 1};
    textureInfo.image.mipLevels = getMipMapLevels(size);
    textureInfo.sampler.maxLod = static_cast<float>(textureInfo.image.mipLevels);
    textureInfo.view.subresourceRange.levelCount = textureInfo.image.mipLevels;

    skyboxTextures.getIrradiance() = vulkan.createTexture(textureInfo);

    transitionTextureDst(vkb, skyboxTextures.getTexture());
    transitionTextureDst(vkb, skyboxTextures.getIrradiance());
    transitionTextureDst(vkb, skyboxTextures.getPrefilter());
}
