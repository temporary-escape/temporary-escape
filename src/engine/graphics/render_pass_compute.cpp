#include "render_pass_compute.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

RenderPassCompute::RenderPassCompute(VulkanRenderer& vulkan, RenderResources& resources, AssetsManager& assetsManager) :
    RenderPass{vulkan, {0, 0}}, subpassCompute{vulkan, resources, assetsManager} {

    logger.info("Creating render pass: {} viewport: {}", typeid(*this).name(), viewport);

    addSubpass(subpassCompute);
    init(true);
}

void RenderPassCompute::render(VulkanCommandBuffer& vkb, Scene& scene) {
    subpassCompute.render(vkb, scene);
}
