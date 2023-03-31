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
    lastViewportSize{viewport} {

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
        renderPasses.opaque = std::make_unique<RenderPassOpaque>(
            vulkan, registry, viewport,
            voxelShapeCache);

        renderPasses.ssao = std::make_unique<RenderPassSsao>(
            vulkan, registry, viewport,
            *renderPasses.opaque);

        renderPasses.lighting = std::make_unique<RenderPassLighting>(
            vulkan, registry, viewport,
            *renderPasses.opaque,
            *renderPasses.ssao,
            renderPasses.brdf->getTexture(RenderPassBrdf::Attachments::Color));

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
    renderPasses.opaque->render(vkb, viewport, scene);
    renderPasses.ssao->render(vkb, viewport, scene);
    renderPasses.lighting->render(vkb, viewport, scene);
    renderPasses.forward->render(vkb, viewport, scene);
    renderPasses.fxaa->render(vkb, viewport, scene);
    renderPasses.bloom->render(vkb, viewport, scene);
    renderPasses.combine->render(vkb, viewport, scene);
    renderPasses.nonHdr->render(vkb, viewport, scene);
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
