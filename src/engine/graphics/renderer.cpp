#include "renderer.hpp"
#include "../assets/assets_manager.hpp"
#include "../utils/exceptions.hpp"
#include "mesh_utils.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

Renderer::Renderer(const Config& config, const Vector2i& viewport, VulkanRenderer& vulkan, Canvas& canvas,
                   Nuklear& nuklear, VoxelShapeCache& voxelShapeCache, AssetsManager& assetsManager, FontFamily& font) :
    config{config},
    vulkan{vulkan},
    canvas{canvas},
    nuklear{nuklear},
    voxelShapeCache{voxelShapeCache},
    assetsManager{assetsManager},
    font{font},
    lastViewportSize{viewport} {

    createRenderPasses(viewport);
    renderBrdf();

    fullScreenQuad = createFullScreenQuad(vulkan);
}

Renderer::~Renderer() = default;

void Renderer::createRenderPasses(const Vector2i& viewport) {
    try {
        if (!renderPasses.brdf) {
            const auto brdfSize = Vector2i{config.graphics.brdfSize, config.graphics.brdfSize};
            renderPasses.brdf = std::make_unique<RenderPassBrdf>(vulkan, assetsManager, brdfSize);
        }

        if (!renderPasses.compute) {
            renderPasses.compute = std::make_unique<RenderPassCompute>(vulkan, assetsManager);
        }

        // clang-format off
        renderPasses.skybox = std::make_unique<RenderPassSkybox>(
            vulkan, assetsManager, viewport,
            renderPasses.brdf->getTexture(RenderPassBrdf::Attachments::Color));

        renderPasses.opaque = std::make_unique<RenderPassOpaque>(
            vulkan, assetsManager, viewport,
            voxelShapeCache, renderPasses.skybox->getTexture(RenderPassSkybox::Depth));

        renderPasses.ssao = std::make_unique<RenderPassSsao>(
            vulkan, assetsManager, viewport,
            *renderPasses.opaque);

        renderPasses.lighting = std::make_unique<RenderPassLighting>(
            vulkan, assetsManager, viewport,
            *renderPasses.opaque,
            *renderPasses.ssao,
            renderPasses.brdf->getTexture(RenderPassBrdf::Attachments::Color),
            renderPasses.skybox->getTexture(RenderPassSkybox::Forward));

        renderPasses.forward = std::make_unique<RenderPassForward>(
            vulkan, assetsManager, viewport,
            *renderPasses.opaque,
            *renderPasses.lighting);

        renderPasses.fxaa = std::make_unique<RenderPassFxaa>(
            vulkan, assetsManager, viewport,
            renderPasses.forward->getTexture(RenderPassForward::Attachments::Forward));

        renderPasses.bloom = std::make_unique<RenderPassBloom>(
            vulkan, assetsManager, viewport,
            renderPasses.fxaa->getTexture(RenderPassFxaa::Attachments::Color));

        renderPasses.combine = std::make_unique<RenderPassCombine>(
            config, vulkan, assetsManager, viewport,
            renderPasses.forward->getTexture(RenderPassForward::Attachments::Forward),
            renderPasses.fxaa->getTexture(RenderPassFxaa::Attachments::Color),
            renderPasses.bloom->getBluredTexture());

        renderPasses.nonHdr = std::make_unique<RenderPassNonHdr>(
            vulkan, assetsManager, viewport,
            *renderPasses.forward);
        // clang-format on
    } catch (...) {
        EXCEPTION_NESTED("Failed to initialize render passes");
    }

    createPipelineBlit();
}

void Renderer::createPipelineBlit() {
    pipelineBlit = std::make_unique<RenderPipeline>(
        vulkan,
        std::vector<ShaderPtr>({
            assetsManager.getShaders().find("blit_vert"),
            assetsManager.getShaders().find("blit_frag"),
        }),
        std::vector<RenderPipeline::VertexInput>({RenderPipeline::VertexInput::of<FullScreenVertex>(0)}),
        RenderPipeline::Options{
            // Additional pipeline options
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            RenderPipeline::DepthMode::Ignore,
            RenderPipeline::Blending::None,
            VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_BACK_BIT,
            VK_FRONT_FACE_CLOCKWISE,
        });
    pipelineBlit->init(vulkan.getRenderPass(), {0}, {});
}

void Renderer::render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene) {
    if (viewport != lastViewportSize) {
        lastViewportSize = viewport;
        vulkan.waitQueueIdle();
        vulkan.waitDeviceIdle();
        createRenderPasses(viewport);
    }

    auto camera = scene.getPrimaryCamera();
    if (!camera) {
        EXCEPTION("Scene has no camera");
    }
    camera->recalculate(vulkan, viewport);

    renderPasses.compute->render(vkb, scene);
    renderPasses.skybox->render(vkb, viewport, scene);
    renderPasses.opaque->render(vkb, viewport, scene);
    renderPasses.ssao->render(vkb, viewport, scene);
    renderPasses.lighting->render(vkb, viewport, scene);
    renderPasses.forward->render(vkb, viewport, scene);
    renderPasses.fxaa->render(vkb, viewport, scene);
    renderPasses.bloom->render(vkb, viewport, scene);
    renderPasses.combine->render(vkb, viewport, scene);
    renderPasses.nonHdr->render(vkb, viewport, scene);
}

void Renderer::renderOneTime(Scene& scene) {
    auto vkb = vulkan.createCommandBuffer();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkb.start(beginInfo);

    render(vkb, lastViewportSize, scene);

    const auto& texture = getTexture();

    auto barrier = VkImageMemoryBarrier{};
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
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkb.pipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, barrier);

    vkb.end();

    vulkan.submitCommandBuffer(vkb);
    vulkan.waitQueueIdle();
}

const VulkanTexture& Renderer::getTexture() const {
    return renderPasses.nonHdr->getTexture(RenderPassNonHdr::Attachments::Forward);
}

void Renderer::render(const std::shared_ptr<Block>& block, VoxelShape::Type shape) {
    logger.info("Rendering thumbnail for block: {}", block ? block->getName() : "nullptr");

    Scene scene{};

    { // Sun
        auto sun = scene.createEntity();
        sun->addComponent<ComponentDirectionalLight>(Color4{1.5f, 1.5f, 1.5f, 1.0f});
        sun->addComponent<ComponentTransform>().translate(Vector3{3.0f, 3.0f, 1.0f});
    }

    { // Camera
        auto entity = scene.createEntity();
        auto& transform = entity->addComponent<ComponentTransform>();
        auto& camera = entity->addComponent<ComponentCamera>(transform);
        entity->addComponent<ComponentUserInput>(camera);
        camera.setProjection(19.0f);
        camera.lookAt({3.0f, 3.0f, 3.0f}, {0.0f, 0.0f, 0.0f});
        scene.setPrimaryCamera(entity);
    }

    { // Skybox
        auto entity = scene.createEntity();
        auto& skybox = entity->addComponent<ComponentSkybox>(0);
        auto skyboxTextures = SkyboxTextures{vulkan, Color4{0.05f, 0.05f, 0.05f, 1.0f}};
        skybox.setTextures(vulkan, std::move(skyboxTextures));
    }

    if (block) {
        auto entity = scene.createEntity();
        auto& transform = entity->addComponent<ComponentTransform>();
        auto& grid = entity->addComponent<ComponentGrid>();
        grid.setDirty(true);
        grid.insert(Vector3i{0, 0, 0}, block, 0, 0, shape);
    }

    scene.update(0.1f);

    renderOneTime(scene);

    vulkan.waitQueueIdle();
}

void Renderer::blit(VulkanCommandBuffer& vkb, const Vector2i& viewport) {
    const auto& texture = renderPasses.nonHdr->getTexture(RenderPassNonHdr::Attachments::Forward);

    pipelineBlit->getDescriptorPool().reset();

    pipelineBlit->bind(vkb);

    std::array<SamplerBindingRef, 1> textures{};
    textures[0] = {"colorTexture", texture};
    pipelineBlit->bindDescriptors(vkb, {}, textures, {});

    pipelineBlit->renderMesh(vkb, fullScreenQuad);
}

void Renderer::renderBrdf() {
    auto vkb = vulkan.createCommandBuffer();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkb.start(beginInfo);

    const auto brdfSize = Vector2i{config.graphics.brdfSize, config.graphics.brdfSize};
    renderPasses.brdf->render(vkb, brdfSize);

    vkb.end();
    vulkan.submitCommandBuffer(vkb);
    vulkan.dispose(std::move(vkb));
    vulkan.waitQueueIdle();
}
