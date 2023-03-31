#include "render_pass_compute.hpp"

using namespace Engine;

RenderPassCompute::RenderPassCompute(VulkanRenderer& vulkan, Registry& registry) :
    RenderPass{vulkan, {0, 0}}, subpassCompute{vulkan, registry} {

    addSubpass(subpassCompute);
    init(true);
}

void RenderPassCompute::render(VulkanCommandBuffer& vkb, Scene& scene) {
    subpassCompute.render(vkb, scene);
}
