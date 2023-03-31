#include "render_subpass_forward.hpp"
#include "../assets/registry.hpp"
#include "render_pass_forward.hpp"
#include "skybox.hpp"

using namespace Engine;

RenderSubpassForward::RenderSubpassForward(VulkanRenderer& vulkan, Registry& registry) :
    vulkan{vulkan},
    pipelineDebug{
        vulkan,
        {
            // List of shader modules
            registry.getShaders().find("component-debug.vert"),
            registry.getShaders().find("component-debug.frag"),
        },
        {
            // Vertex inputs
            RenderPipeline::VertexInput::of<ComponentDebug::Vertex>(0),
        },
        {
            // Additional pipeline options
            VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
            RenderPipeline::DepthMode::ReadWrite,
            RenderPipeline::Blending::None,
            VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_BACK_BIT,
            VK_FRONT_FACE_COUNTER_CLOCKWISE,
        },
    } {

    setAttachments({
        RenderPassForward::Attachments::Depth,
        RenderPassForward::Attachments::Forward,
    });

    addPipeline(pipelineDebug);
}

void RenderSubpassForward::render(VulkanCommandBuffer& vkb, Scene& scene) {
    pipelineDebug.getDescriptorPool().reset();

    std::vector<ForwardRenderJob> jobs;
    collectForRender<ComponentDebug>(vkb, scene, jobs);

    std::sort(jobs.begin(), jobs.end(), [](auto& a, auto& b) { return a.order > b.order; });

    currentPipeline = nullptr;
    for (auto& job : jobs) {
        job.fn();
    }
}

void RenderSubpassForward::renderSceneForward(VulkanCommandBuffer& vkb, const ComponentCamera& camera,
                                              ComponentTransform& transform, ComponentDebug& component) {
    component.recalculate(vulkan);

    const auto& mesh = component.getMesh();

    if (mesh.count == 0) {
        return;
    }

    if (currentPipeline != &pipelineDebug) {
        currentPipeline = &pipelineDebug;
        currentPipeline->bind(vkb);
    }

    std::array<UniformBindingRef, 1> uniforms{};
    uniforms[0] = {"Camera", camera.getUbo().getCurrentBuffer()};

    pipelineDebug.bindDescriptors(vkb, uniforms, {}, {});

    const auto modelMatrix = transform.getAbsoluteTransform();
    pipelineDebug.pushConstants(vkb, PushConstant{"modelMatrix", modelMatrix});

    pipelineDebug.renderMesh(vkb, mesh);
}
