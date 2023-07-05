#include "render_subpass_compute.hpp"
#include "../assets/assets_manager.hpp"
#include "../scene/controllers/controller_2d_selectable.hpp"
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

    pipelinePositionFeedback.bind(vkb);

    renderSceneCompute(vkb, camera, scene.getController<Controller2DSelectable>());
}

void RenderSubpassCompute::renderSceneCompute(VulkanCommandBuffer& vkb, const ComponentCamera& camera,
                                              Controller2DSelectable& controller) {
    controller.recalculate(vulkan);

    if (controller.getCount() == 0) {
        return;
    }

    std::array<UniformBindingRef, 3> uniforms{};
    uniforms[0] = {"Camera", camera.getUbo().getCurrentBuffer()};
    uniforms[1] = {"InputBuffer", controller.getBufferInput()};
    uniforms[2] = {"OutputBuffer", controller.getBufferOutput()};

    pipelinePositionFeedback.bindDescriptors(vkb, uniforms, {}, {});

    const auto viewport = Vector2{camera.getViewport()};
    const auto count = static_cast<int>(controller.getCount());
    pipelinePositionFeedback.pushConstants(vkb, PushConstant{"viewport", viewport}, PushConstant{"count", count});

    const uint32_t workCount = (controller.getCount() / 256) + 1;
    vkb.dispatch(workCount, 1, 1);
}
