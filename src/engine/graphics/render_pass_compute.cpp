#include "render_pass_compute.hpp"

using namespace Engine;

static auto logger = createLogger(__FILENAME__);

RenderPassCompute::RenderPassCompute(VulkanRenderer& vulkan, Registry& registry) :
    RenderPass{vulkan, {0, 0}}, subpassCompute{vulkan, registry} {

    logger.info("Creating render pass: {} viewport: {}", typeid(*this).name(), viewport);

    addSubpass(subpassCompute);
    init(true);
}

void RenderPassCompute::render(VulkanCommandBuffer& vkb, Scene& scene) {
    subpassCompute.render(vkb, scene);
}
