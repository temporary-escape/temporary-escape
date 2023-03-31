#include "render_subpass.hpp"

using namespace Engine;

void RenderSubpass::init(VulkanRenderPass& renderPass, uint32_t subpass) {
    for (auto& pipeline : pipelines) {
        pipeline->init(renderPass, attachments, subpass);
    }
}

void RenderSubpass::init() {
    for (auto& pipeline : pipelines) {
        pipeline->init();
    }
}
