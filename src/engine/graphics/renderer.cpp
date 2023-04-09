#include "renderer.hpp"
#include "../assets/registry.hpp"
#include "../utils/exceptions.hpp"
#include "mesh_utils.hpp"

using namespace Engine;

static auto logger = createLogger(__FILENAME__);

Renderer::Renderer(const Config& config, const Vector2i& viewport, VulkanRenderer& vulkan, Canvas& canvas,
                   Nuklear& nuklear, VoxelShapeCache& voxelShapeCache, Registry& registry, FontFamily& font) :
    config{config},
    vulkan{vulkan},
    canvas{canvas},
    nuklear{nuklear},
    voxelShapeCache{voxelShapeCache},
    registry{registry},
    font{font},
    lastViewportSize{viewport},
    skybox{vulkan, Color4{0.05f, 0.05f, 0.05f, 1.0f}} {

    createRenderPasses(viewport);
    renderBrdf();

    fullScreenQuad = createFullScreenQuad(vulkan);
}

Renderer::~Renderer() = default;

void Renderer::createRenderPasses(const Vector2i& viewport) {
    try {
        if (!renderPasses.brdf) {
            const auto brdfSize = Vector2i{config.brdfSize, config.brdfSize};
            renderPasses.brdf = std::make_unique<RenderPassBrdf>(vulkan, registry, brdfSize);
        }

        if (!renderPasses.compute) {
            renderPasses.compute = std::make_unique<RenderPassCompute>(vulkan, registry);
        }

        // clang-format off
        renderPasses.skybox = std::make_unique<RenderPassSkybox>(
            vulkan, registry, viewport,
            renderPasses.brdf->getTexture(RenderPassBrdf::Attachments::Color));

        renderPasses.opaque = std::make_unique<RenderPassOpaque>(
            vulkan, registry, viewport,
            voxelShapeCache, renderPasses.skybox->getTexture(RenderPassSkybox::Depth));

        renderPasses.ssao = std::make_unique<RenderPassSsao>(
            vulkan, registry, viewport,
            *renderPasses.opaque);

        renderPasses.lighting = std::make_unique<RenderPassLighting>(
            vulkan, registry, viewport,
            *renderPasses.opaque,
            *renderPasses.ssao,
            renderPasses.brdf->getTexture(RenderPassBrdf::Attachments::Color),
            renderPasses.skybox->getTexture(RenderPassSkybox::Forward));

        renderPasses.forward = std::make_unique<RenderPassForward>(
            vulkan, registry, viewport,
            *renderPasses.opaque,
            *renderPasses.lighting);

        renderPasses.fxaa = std::make_unique<RenderPassFxaa>(
            vulkan, registry, viewport,
            renderPasses.forward->getTexture(RenderPassForward::Attachments::Forward));

        renderPasses.bloom = std::make_unique<RenderPassBloom>(
            vulkan, registry, viewport,
            renderPasses.fxaa->getTexture(RenderPassFxaa::Attachments::Color));

        renderPasses.combine = std::make_unique<RenderPassCombine>(
            config, vulkan, registry, viewport,
            renderPasses.forward->getTexture(RenderPassForward::Attachments::Forward),
            renderPasses.fxaa->getTexture(RenderPassFxaa::Attachments::Color),
            renderPasses.bloom->getBluredTexture());

        renderPasses.nonHdr = std::make_unique<RenderPassNonHdr>(
            vulkan, registry, viewport,
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
            registry.getShaders().find("blit.vert"),
            registry.getShaders().find("blit.frag"),
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

    auto sun = scene.createEntity();
    sun->addComponent<ComponentDirectionalLight>(Color4{1.5f, 1.5f, 1.5f, 1.0f});
    sun->addComponent<ComponentTransform>().translate(Vector3{3.0f, 3.0f, 1.0f});

    auto entityCamera = scene.createEntity();
    auto& cameraTransform = entityCamera->addComponent<ComponentTransform>();
    auto& cameraCamera = entityCamera->addComponent<ComponentCamera>(cameraTransform);
    entityCamera->addComponent<ComponentUserInput>(cameraCamera);
    cameraCamera.setProjection(19.0f);
    cameraCamera.lookAt({3.0f, 3.0f, 3.0f}, {0.0f, 0.0f, 0.0f});
    scene.setPrimaryCamera(entityCamera);

    scene.setSkybox(skybox);

    if (block) {
        auto entityBlock = scene.createEntity();
        auto& entityTransform = entityBlock->addComponent<ComponentTransform>();
        auto& grid = entityBlock->addComponent<ComponentGrid>();
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

    const auto brdfSize = Vector2i{config.brdfSize, config.brdfSize};
    renderPasses.brdf->render(vkb, brdfSize);

    vkb.end();
    vulkan.submitCommandBuffer(vkb);
    vulkan.dispose(std::move(vkb));
    vulkan.waitQueueIdle();
}
