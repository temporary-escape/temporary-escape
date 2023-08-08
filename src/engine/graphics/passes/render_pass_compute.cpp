#include "render_pass_compute.hpp"
#include "../../assets/assets_manager.hpp"
#include "../../scene/controllers/controller_icon.hpp"
#include "../../scene/controllers/controller_icon_selectable.hpp"
#include "../../scene/scene.hpp"

using namespace Engine;

RenderPassCompute::RenderPassCompute(const RenderOptions& options, VulkanRenderer& vulkan, RenderBufferPbr& buffer,
                                     AssetsManager& assetsManager) :
    RenderPass{vulkan, buffer, "RenderPassCompute"}, pipelinePositionFeedback{vulkan, assetsManager} {

    addPipeline(pipelinePositionFeedback, 0);
}

void RenderPassCompute::render(VulkanCommandBuffer& vkb, Scene& scene) {
    renderPositionFeedback(vkb, scene);
}

void RenderPassCompute::renderPositionFeedback(VulkanCommandBuffer& vkb, Scene& scene) {
    auto& camera = *scene.getPrimaryCamera();
    auto& controller = scene.getController<ControllerIconSelectable>();

    std::array<const VulkanArrayBuffer*, 2> inputBuffers{};
    inputBuffers[0] = &controller.getDynamicBufferInput();
    inputBuffers[1] = &controller.getStaticBufferInput();

    std::array<const VulkanDoubleBuffer*, 2> outputBuffers{};
    outputBuffers[0] = &controller.getDynamicBufferOutput();
    outputBuffers[1] = &controller.getStaticBufferOutput();

    if (inputBuffers[0]->empty() && inputBuffers[1]->empty()) {
        return;
    }

    pipelinePositionFeedback.bind(vkb);

    for (size_t i = 0; i < 2; i++) {
        if (inputBuffers[i]->empty() || !*outputBuffers[i]) {
            continue;
        }

        std::array<UniformBindingRef, 3> uniforms{};
        uniforms[0] = {"Camera", camera.getUbo().getCurrentBuffer()};
        uniforms[1] = {"InputBuffer", inputBuffers[i]->getCurrentBuffer()};
        uniforms[2] = {"OutputBuffer", outputBuffers[i]->getCurrentBuffer()};

        pipelinePositionFeedback.setUniformCamera(camera.getUbo().getCurrentBuffer());
        pipelinePositionFeedback.setBufferInput(inputBuffers[i]->getCurrentBuffer());
        pipelinePositionFeedback.setBufferOutput(outputBuffers[i]->getCurrentBuffer());
        pipelinePositionFeedback.flushDescriptors(vkb);

        const auto viewport = Vector2{camera.getViewport()};
        const auto count = static_cast<int>(inputBuffers[i]->count());
        pipelinePositionFeedback.setCount(count);
        pipelinePositionFeedback.setViewport(viewport);
        pipelinePositionFeedback.flushConstants(vkb);

        const uint32_t workCount = (inputBuffers[i]->count() / 256) + 1;
        vkb.dispatch(workCount, 1, 1);
    }
}
