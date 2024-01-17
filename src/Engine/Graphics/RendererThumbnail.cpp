#include "RendererThumbnail.hpp"
#include "../Scene/Scene.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

static RenderOptions getOptionsThumbnails(const Config& config) {
    RenderOptions options{};
    options.fxaa = true;
    options.ssao = false;
    options.bloom = false;
    options.shadowsSize = 0;
    options.shadowsLevel = 0;
    options.viewport = {config.thumbnailSize, config.thumbnailSize};
    options.enableSrc = true;
    return options;
}

RendererThumbnail::RendererThumbnail(const Config& config, VulkanRenderer& vulkan, RenderResources& resources,
                                     VoxelShapeCache& voxelShapeCache) :
    RendererScenePbr{getOptionsThumbnails(config), vulkan, resources},
    config{config},
    vulkan{vulkan},
    voxelShapeCache{voxelShapeCache} {
}

void RendererThumbnail::renderOneTime(Scene& scene) {
    vkb = vulkan.createCommandBuffer();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkb.start(beginInfo);

    RendererScenePbr::render(vkb, scene);

    const auto& texture = getFinalBuffer();

    auto barrier = VkImageMemoryBarrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = texture.getHandle();
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = texture.getMipMaps();
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkb.pipelineBarrier(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, barrier);

    vkb.end();

    vulkan.submitCommandBuffer(vkb);
    vulkan.waitQueueIdle();
}

void RendererThumbnail::render(const Engine::BlockPtr& block, const Engine::VoxelShape::Type shape) {
    logger.info("Rendering thumbnail for block: {}", block ? block->getName() : "nullptr");

    Scene scene{config, &voxelShapeCache};

    { // Sun
        auto sun = scene.createEntity();
        sun.addComponent<ComponentDirectionalLight>(Color4{1.5f, 1.5f, 1.5f, 1.0f});
        sun.addComponent<ComponentTransform>().translate(Vector3{3.0f, 3.0f, 1.0f});
    }

    { // Camera
        auto entity = scene.createEntity();
        auto& transform = entity.addComponent<ComponentTransform>();
        auto& camera = entity.addComponent<ComponentCamera>(transform);
        camera.setProjection(19.0f);
        camera.lookAt({3.0f, 3.0f, 3.0f}, {0.0f, 0.0f, 0.0f});
        scene.setPrimaryCamera(entity);
    }

    { // Skybox
        auto entity = scene.createEntity();
        auto& skybox = entity.addComponent<ComponentSkybox>(0);
        auto skyboxTextures = SkyboxTextures{vulkan, Color4{0.02f, 0.02f, 0.02f, 1.0f}};
        skybox.setTextures(vulkan, std::move(skyboxTextures));
    }

    if (block) {
        auto entity = scene.createEntity();
        entity.addComponent<ComponentTransform>();
        auto& grid = entity.addComponent<ComponentGrid>();
        grid.setDirty();
        grid.insert(Vector3i{0, 0, 0}, block, 0, 1, shape);
    }

    scene.update(0.1f);

    renderOneTime(scene);

    vulkan.waitQueueIdle();
}

void RendererThumbnail::render(const PlanetTypePtr& planetType) {
    logger.info("Rendering thumbnail for planet: {}", planetType->getName());

    if (!planetType->getLowResTextures().getColor()) {
        EXCEPTION("Planet type: {} has no textures generated", planetType->getName());
    }

    Scene scene{config, &voxelShapeCache};

    { // Sun
        auto sun = scene.createEntity();
        sun.addComponent<ComponentDirectionalLight>(Color4{1.0f, 1.0f, 1.0f, 1.0f});
        sun.addComponent<ComponentTransform>().translate(Vector3{-3.0f, 1.0f, 1.0f});
    }

    { // Camera
        auto entity = scene.createEntity();
        auto& transform = entity.addComponent<ComponentTransform>();
        auto& camera = entity.addComponent<ComponentCamera>(transform);
        camera.setProjection(40.0f);
        camera.lookAt({0.0f, 0.0f, 3.0f}, {0.0f, 0.0f, 0.0f});
        scene.setPrimaryCamera(entity);
    }

    { // Skybox
        auto entity = scene.createEntity();
        auto& skybox = entity.addComponent<ComponentSkybox>(0);
        auto skyboxTextures = SkyboxTextures{vulkan, Color4{0.02f, 0.02f, 0.02f, 1.0f}};
        skybox.setTextures(vulkan, std::move(skyboxTextures));
    }

    { // Planet
        auto entity = scene.createEntity();
        auto& transform = entity.addComponent<ComponentTransform>();
        transform.translate(Vector3{0.0f, 0.0f, -2.0f});
        auto& planet = entity.addComponent<ComponentPlanet>(planetType, 1234);
        planet.setBackground(true);
        planet.setHighRes(false);
    }

    scene.update(0.1f);

    renderOneTime(scene);

    vulkan.waitQueueIdle();
}
