#include "render_subpass_non_hdr.hpp"
#include "../assets/assets_manager.hpp"
#include "../scene/controllers/controller_icon.hpp"
#include "render_pass_non_hdr.hpp"
#include "skybox.hpp"

using namespace Engine;

RenderSubpassNonHdr::RenderSubpassNonHdr(VulkanRenderer& vulkan, RenderResources& resources,
                                         AssetsManager& assetsManager) :
    vulkan{vulkan},
    resources{resources},
    pipelineWorldText{
        vulkan,
        {
            // List of shader modules
            assetsManager.getShaders().find("component_world_text_vert"),
            assetsManager.getShaders().find("component_world_text_frag"),
        },
        {
            // Vertex inputs
            RenderPipeline::VertexInput::of<ComponentWorldText::Vertex>(0, VK_VERTEX_INPUT_RATE_INSTANCE),
        },
        {
            // Additional pipeline options
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
            RenderPipeline::DepthMode::Read,
            RenderPipeline::Blending::Normal,
            VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_BACK_BIT,
            VK_FRONT_FACE_COUNTER_CLOCKWISE,
        },
    },
    pipelinePointCloud{
        vulkan,
        {
            // List of shader modules
            assetsManager.getShaders().find("component_point_cloud_vert"),
            assetsManager.getShaders().find("component_point_cloud_frag"),
        },
        {
            // Vertex inputs
            RenderPipeline::VertexInput::of<ComponentPointCloud::Point>(0, VK_VERTEX_INPUT_RATE_INSTANCE),
        },
        {
            // Additional pipeline options
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
            RenderPipeline::DepthMode::Ignore,
            RenderPipeline::Blending::Normal,
            VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_BACK_BIT,
            VK_FRONT_FACE_COUNTER_CLOCKWISE,
        },
    } {

    setAttachments({
        RenderPassNonHdr::Attachments::Depth,
        RenderPassNonHdr::Attachments::Forward,
    });

    addPipeline(pipelineWorldText);
    addPipeline(pipelinePointCloud);
}

void RenderSubpassNonHdr::render(VulkanCommandBuffer& vkb, Scene& scene) {
    pipelineWorldText.getDescriptorPool().reset();
    pipelinePointCloud.getDescriptorPool().reset();

    std::vector<ForwardRenderJob> jobs;
    collectForRender<ComponentWorldText>(vkb, scene, jobs);

    std::sort(jobs.begin(), jobs.end(), [](auto& a, auto& b) { return a.order > b.order; });

    currentPipeline = nullptr;
    for (auto& job : jobs) {
        job.fn();
    }

    auto& camera = *scene.getPrimaryCamera();
    renderSceneForward(vkb, camera, scene.getController<ControllerIcon>());
}

void RenderSubpassNonHdr::renderSceneForward(VulkanCommandBuffer& vkb, const ComponentCamera& camera,
                                             ComponentTransform& transform, ComponentWorldText& component) {
    component.recalculate(vulkan);

    const auto& mesh = component.getMesh();

    if (mesh.count == 0) {
        return;
    }

    if (currentPipeline != &pipelineWorldText) {
        currentPipeline = &pipelineWorldText;
        currentPipeline->bind(vkb);
    }

    std::array<UniformBindingRef, 1> uniforms{};
    uniforms[0] = {"Camera", camera.getUbo().getCurrentBuffer()};

    std::array<SamplerBindingRef, 1> textures{};
    textures[0] = {"colorTexture", component.getFontFace().getTexture()};

    pipelineWorldText.bindDescriptors(vkb, uniforms, textures, {});

    const auto modelMatrix = transform.getAbsoluteTransform();
    const auto color = component.getColor();
    pipelineWorldText.pushConstants(vkb, PushConstant{"modelMatrix", modelMatrix}, PushConstant{"color", color});

    pipelineWorldText.renderMesh(vkb, mesh);
}

void RenderSubpassNonHdr::renderSceneForward(VulkanCommandBuffer& vkb, const ComponentCamera& camera,
                                             ControllerIcon& controller) {
    controller.recalculate(vulkan);

    if (currentPipeline != &pipelinePointCloud) {
        currentPipeline = &pipelinePointCloud;
        currentPipeline->bind(vkb);
    }

    const auto& vbos = controller.getVbos();
    const auto& counts = controller.getCounts();

    const auto modelMatrix = Matrix4{1.0f};
    pipelinePointCloud.pushConstants(vkb, PushConstant{"modelMatrix", modelMatrix});

    for (const auto& pair : vbos) {
        const auto count = counts.at(pair.first);

        if (count == 0) {
            continue;
        }

        std::array<UniformBindingRef, 1> uniforms{};
        uniforms[0] = {"Camera", camera.getUbo().getCurrentBuffer()};

        std::array<SamplerBindingRef, 1> textures{};
        textures[0] = {"colorTexture", *pair.first->getAllocation().texture};

        pipelinePointCloud.bindDescriptors(vkb, uniforms, textures, {});

        std::array<VulkanVertexBufferBindRef, 1> vboBindings{};
        vboBindings[0] = {&pair.second.getCurrentBuffer(), 0};
        vkb.bindBuffers(vboBindings);

        vkb.draw(4, count, 0, 0);
    }
}
