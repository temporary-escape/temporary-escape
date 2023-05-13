#include "render_subpass_compute.hpp"
#include "../assets/registry.hpp"
#include "skybox.hpp"

using namespace Engine;

RenderSubpassCompute::RenderSubpassCompute(VulkanRenderer& vulkan, Registry& registry) :
    vulkan{vulkan},
    pipelinePositionFeedback{
        vulkan,
        {
            // List of shader modules
            registry.getShaders().find("position_feedback_comp"),
        },
    } {

    addPipeline(pipelinePositionFeedback);
}

void RenderSubpassCompute::render(VulkanCommandBuffer& vkb, Scene& scene) {
    auto& camera = *scene.getPrimaryCamera();

    pipelinePositionFeedback.getDescriptorPool().reset();

    pipelinePositionFeedback.bind(vkb);
    for (auto&& [entity, transform, component] : scene.getView<ComponentTransform, ComponentClickablePoints>().each()) {
        renderSceneCompute(vkb, camera, transform, component);
    }
}

void RenderSubpassCompute::renderSceneCompute(VulkanCommandBuffer& vkb, const ComponentCamera& camera,
                                              ComponentTransform& transform, ComponentClickablePoints& component) {
    component.recalculate(vulkan);

    std::array<UniformBindingRef, 3> uniforms{};
    uniforms[0] = {"Camera", camera.getUbo().getCurrentBuffer()};
    uniforms[1] = {"InputBuffer", component.getBufferInput()};
    uniforms[2] = {"OutputBuffer", component.getBufferOutput()};

    pipelinePositionFeedback.bindDescriptors(vkb, uniforms, {}, {});

    const auto modelMatrix = transform.getAbsoluteTransform();
    const auto viewport = Vector2{camera.getViewport()};
    const auto count = static_cast<int>(component.getCount());
    pipelinePositionFeedback.pushConstants(vkb,
                                           PushConstant{"modelMatrix", modelMatrix},
                                           PushConstant{"viewport", viewport},
                                           PushConstant{"count", count});

    const uint32_t workCount = (component.getCount() / 256) + 1;
    vkb.dispatch(workCount, 1, 1);
}
