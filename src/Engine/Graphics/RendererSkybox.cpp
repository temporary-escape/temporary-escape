#include "RendererSkybox.hpp"
#include "../Scene/Scene.hpp"
#include "../Utils/Exceptions.hpp"
#include "Passes/RenderPassSkyboxColor.hpp"
#include "Passes/RenderPassSkyboxIrradiance.hpp"
#include "Passes/RenderPassSkyboxPrefilter.hpp"

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

RendererSkybox::RendererSkybox(const Config& config, VulkanRenderer& vulkan, RenderResources& resources,
                               VoxelShapeCache& voxelShapeCache) :
    RendererWork{config, vulkan, voxelShapeCache},
    config{config},
    vulkan{vulkan},
    resources{resources},
    renderBufferSkybox{config, vulkan} {

    try {
        addRenderPass(std::make_unique<RenderPassSkyboxColor>(vulkan, renderBufferSkybox, resources));
        addRenderPass(std::make_unique<RenderPassSkyboxIrradiance>(vulkan, renderBufferSkybox, resources));
        addRenderPass(std::make_unique<RenderPassSkyboxPrefilter>(vulkan, renderBufferSkybox, resources));
    } catch (...) {
        EXCEPTION_NESTED("Failed to setup render passes");
    }
    create();
}

void RendererSkybox::update(Scene& scene) {
    if (isBusy()) {
        return;
    }

    auto skyboxes = scene.getView<ComponentSkybox>();
    for (auto&& [handle, skybox] : skyboxes.each()) {
        if (!skybox.isGenerated()) {
            logger.info("Starting rendering skybox seed: {}", skybox.getSeed());

            work.entity = scene.fromHandle(handle);
            work.seed = skybox.getSeed();
            startJobs(12);
            break;
        }
    }
}

void RendererSkybox::finished() {
    logger.info("Finished rendering skybox seed: {}", work.seed);

    work.entity.getComponent<ComponentSkybox>().setTextures(vulkan, std::move(skyboxTextures));
    skyboxTextures = SkyboxTextures{};
}

void RendererSkybox::beforeRender(VulkanCommandBuffer& vkb, Scene& scene, const size_t job) {
    auto& passSkyboxColor = getRenderPass<RenderPassSkyboxIrradiance>();
    auto& passSkyboxIrradiance = getRenderPass<RenderPassSkyboxIrradiance>();
    auto& passSkyboxPrefilter = getRenderPass<RenderPassSkyboxPrefilter>();

    auto camera = scene.getPrimaryCamera();
    if (!camera) {
        EXCEPTION("Scene has no camera");
    }

    // Color render step
    if (job < 6) {
        passSkyboxColor.setExcluded(false);
        passSkyboxIrradiance.setExcluded(true);
        passSkyboxPrefilter.setExcluded(true);

        if (job == 0) {
            prepareCubemap(vkb);
        }

        prepareProperties(scene);

        camera->lookAt({0.0f, 0.0f, 0.0f}, captureViewCenter[job % 6], captureViewUp[job % 6]);
        camera->setProjection(90.0f);
        camera->setViewport({1.0f, 1.0f});
    }
    // Post process
    else {
        passSkyboxColor.setExcluded(true);
        passSkyboxIrradiance.setExcluded(false);
        passSkyboxPrefilter.setExcluded(false);

        camera->lookAt({0.0f, 0.0f, 0.0f}, captureViewCenter[job % 6], captureViewUp[job % 6]);
        camera->setProjectionMatrix(captureProjectionMatrix);
        camera->setViewport({1.0f, 1.0f});

        passSkyboxIrradiance.setTextureSkybox(skyboxTextures.getTexture());
        passSkyboxPrefilter.setTextureSkybox(skyboxTextures.getTexture());
    }
}

void RendererSkybox::postRender(VulkanCommandBuffer& vkb, Scene& scene, const size_t job) {
    // Color render step
    if (job < 6) {
        copyTexture(vkb, RenderBufferSkybox::Attachment::Color, skyboxTextures.getTexture(), job % 6);

        if (job == 5) {
            vkb.generateMipMaps(skyboxTextures.getTexture());
        }
    }
    // Post process
    else {
        copyTexture(vkb, RenderBufferSkybox::Attachment::Irradiance, skyboxTextures.getIrradiance(), job % 6);
        copyMipMaps(vkb, RenderBufferSkybox::Attachment::Irradiance, skyboxTextures.getIrradiance(), job % 6);

        copyTexture(vkb, RenderBufferSkybox::Attachment::Prefilter, skyboxTextures.getPrefilter(), job % 6);
        copyMipMaps(vkb, RenderBufferSkybox::Attachment::Prefilter, skyboxTextures.getPrefilter(), job % 6);

        if (job == 11) {
            transitionTextureRead(vkb, skyboxTextures.getIrradiance());
            transitionTextureRead(vkb, skyboxTextures.getPrefilter());
        }
    }
}

void RendererSkybox::prepareProperties(Scene& scene) {
    Rng rng{work.seed};
    prepareStars(scene, rng, {0.3f, 0.3f}, 100000);
    prepareStars(scene, rng, {0.5f, 0.5f}, 1000);
    prepareNebulas(scene, rng);
}

void RendererSkybox::prepareNebulas(Scene& scene, Rng& rng) const {
    std::uniform_real_distribution<float> dist{0.0f, 1.0f};

    while (true) {
        const auto scale = dist(rng) * 0.5f + 0.25f;
        const auto intensity = dist(rng) * 0.2f + 0.9f;
        const auto color = Color4{dist(rng), dist(rng), dist(rng), 1.0f};
        const auto falloff = dist(rng) * 3.0f + 3.0f;
        const auto offset =
            Vector3{dist(rng) * 2000.0f - 1000.0f, dist(rng) * 2000.0f - 1000.0f, dist(rng) * 2000.0f - 1000.0f};

        auto entity = scene.createEntity();
        entity.addComponent<ComponentTransform>();
        auto& nebula = entity.addComponent<ComponentNebula>();

        nebula.setColor(color);
        nebula.setOffset({offset, 1.0f});
        nebula.setScale(scale);
        nebula.setIntensity(intensity * 0.95f);
        nebula.setFalloff(falloff);

        if (dist(rng) < 0.5f) {
            break;
        }
    }
}

void RendererSkybox::prepareStars(Scene& scene, Rng& rng, const Vector2& size, const size_t count) {
    logger.debug("Preparing skybox vertex buffer for: {} stars", count);

    std::uniform_real_distribution<float> distUV{0.0f, 1.0f};
    std::uniform_real_distribution<float> distColor(0.9f, 1.0f);
    std::uniform_real_distribution<float> distBrightness(0.2f, 1.0f);

    auto entity = scene.createEntity();
    entity.addComponent<ComponentTransform>();
    auto& pointCloud = entity.addComponent<ComponentPointCloud>(resources.getSkyboxStar());
    pointCloud.reserve(count);

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
        pointCloud.add(position * 100.0f, size, color);
    }
}

void RendererSkybox::copyTexture(VulkanCommandBuffer& vkb, const uint32_t attachment, const VulkanTexture& target,
                                 const int side) {
    /*renderBufferSkybox.transitionLayout(vkb,
                                        attachment,
                                        VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                        VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT,
                                        VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT);*/

    const auto& source = renderBufferSkybox.getAttachmentTexture(attachment);
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
        static_cast<int32_t>(source.getExtent().height), // This is on purpose!
        static_cast<int32_t>(source.getExtent().height),
        1,
    };

    std::array<VkImageBlit, 1> blit{};
    blit[0].srcOffsets[0] = {0, 0, 0};
    blit[0].srcOffsets[1] = offset;
    blit[0].srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit[0].srcSubresource.mipLevel = 0;
    blit[0].srcSubresource.baseArrayLayer = 0;
    blit[0].srcSubresource.layerCount = source.getLayerCount();
    blit[0].dstOffsets[0] = {0, 0, 0};
    blit[0].dstOffsets[1] = offset;
    blit[0].dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit[0].dstSubresource.mipLevel = 0;
    blit[0].dstSubresource.baseArrayLayer = side;
    blit[0].dstSubresource.layerCount = source.getLayerCount();

    vkb.blitImage(source,
                  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                  target,
                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                  blit,
                  VK_FILTER_NEAREST);
}

void RendererSkybox::copyMipMaps(VulkanCommandBuffer& vkb, uint32_t attachment, const VulkanTexture& target,
                                 const int side) const {
    const auto& source = renderBufferSkybox.getAttachmentTexture(attachment);

    Vector2i size = {
        source.getExtent().height, // This is on purpose!
        source.getExtent().height,
    };

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

void RendererSkybox::transitionTextureRead(VulkanCommandBuffer& vkb, const VulkanTexture& texture) const {
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

void RendererSkybox::prepareTexture(VulkanCommandBuffer& vkb, const VulkanTexture& target) {
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

void RendererSkybox::prepareCubemap(VulkanCommandBuffer& vkb) {
    skyboxTextures.dispose(vulkan);
    skyboxTextures = SkyboxTextures{};

    auto size = Vector2i{config.graphics.skyboxSize, config.graphics.skyboxSize};

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
    textureInfo.view.subresourceRange.levelCount = getMipMapLevels(size);
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

    prepareTexture(vkb, skyboxTextures.getTexture());
    prepareTexture(vkb, skyboxTextures.getIrradiance());
    prepareTexture(vkb, skyboxTextures.getPrefilter());
}
