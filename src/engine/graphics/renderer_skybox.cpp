#include "renderer_skybox.hpp"
#include "../scene/scene.hpp"
#include "../utils/exceptions.hpp"
#include "passes/render_pass_skybox_color.hpp"

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

static RenderOptions getRenderOptions(const Config& config) {
    RenderOptions options{};
    options.viewport = {config.graphics.skyboxSize, config.graphics.skyboxSize};
    return options;
}

RendererSkybox::RendererSkybox(const Config& config, VulkanRenderer& vulkan, RenderResources& resources,
                               AssetsManager& assetsManager) :
    RendererWork{config, getRenderOptions(config), vulkan},
    config{config},
    vulkan{vulkan},
    renderBufferSkybox{getRenderOptions(config), vulkan} {

    try {
        addRenderPass(std::make_unique<RenderPassSkyboxColor>(vulkan, renderBufferSkybox, resources, assetsManager));
    } catch (...) {
        EXCEPTION_NESTED("Failed to setup render passes");
    }
    create();

    textures.star = assetsManager.getTextures().find("system_map_sun");
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
            startJobs(6);
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
    if (job == 0) {
        prepareCubemap(vkb);
        prepareProperties(scene);
    }

    auto camera = scene.getPrimaryCamera();
    if (!camera) {
        EXCEPTION("Scene has no camera");
    }
    camera->lookAt({0.0f, 0.0f, 0.0f}, captureViewCenter[job], captureViewUp[job]);
    camera->setProjection(90.0f);
}

void RendererSkybox::postRender(VulkanCommandBuffer& vkb, Scene& scene, const size_t job) {
    copyTexture(vkb, RenderBufferSkybox::Attachment::Color, skyboxTextures.getTexture(), job);

    if (job == 5) {
        vkb.generateMipMaps(skyboxTextures.getTexture());
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
        const auto intensity = dist(rng) * 0.5f + 0.5f;
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
        nebula.setIntensity(intensity * 0.8f);
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
    auto& pointCloud = entity.addComponent<ComponentPointCloud>(textures.star);

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
                                 int side) {
    renderBufferSkybox.transitionLayout(vkb,
                                        attachment,
                                        VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                        VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT,
                                        VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT);

    const auto& source = renderBufferSkybox.getAttachmentTexture(attachment);

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
