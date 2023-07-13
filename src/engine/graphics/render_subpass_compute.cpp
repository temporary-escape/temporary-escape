#include "render_subpass_compute.hpp"
#include "../assets/assets_manager.hpp"
#include "../scene/controllers/controller_icon_selectable.hpp"
#include "skybox.hpp"

using namespace Engine;

RenderSubpassCompute::RenderSubpassCompute(VulkanRenderer& vulkan, RenderResources& resources,
                                           AssetsManager& assetsManager) :
    vulkan{vulkan},
    resources{resources},
    pipelinePositionFeedback{
        vulkan,
        {
            // List of shader modules
            assetsManager.getShaders().find("position_feedback_comp"),
        },
    } {

    addPipeline(pipelinePositionFeedback);
}

void RenderSubpassCompute::render(VulkanCommandBuffer& vkb, Scene& scene) {
    auto& camera = *scene.getPrimaryCamera();

    pipelinePositionFeedback.getDescriptorPool().reset();

    renderSceneCompute(vkb, camera, scene.getController<ControllerIconSelectable>());
}

void RenderSubpassCompute::renderSceneCompute(VulkanCommandBuffer& vkb, const ComponentCamera& camera,
                                              ControllerIconSelectable& controller) {
    controller.recalculate(vulkan);

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

        pipelinePositionFeedback.bindDescriptors(vkb, uniforms, {}, {});

        const auto viewport = Vector2{camera.getViewport()};
        const auto count = static_cast<int>(inputBuffers[i]->count());
        pipelinePositionFeedback.pushConstants(vkb, PushConstant{"viewport", viewport}, PushConstant{"count", count});

        const uint32_t workCount = (inputBuffers[i]->count() / 256) + 1;
        vkb.dispatch(workCount, 1, 1);
    }
}
