#include "render_subpass_non_hdr.hpp"
#include "../assets/assets_manager.hpp"
#include "../scene/controllers/controller_icon.hpp"
#include "mesh_utils.hpp"
#include "render_pass_non_hdr.hpp"
#include "skybox.hpp"

using namespace Engine;

RenderSubpassNonHdr::RenderSubpassNonHdr(VulkanRenderer& vulkan, RenderResources& resources,
                                         AssetsManager& assetsManager, const VulkanTexture& outlineTexture) :
    vulkan{vulkan},
    resources{resources},
    outlineTexture{outlineTexture},
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
    pipelineIcons{
        vulkan,
        {
            // List of shader modules
            assetsManager.getShaders().find("component_icons_vert"),
            assetsManager.getShaders().find("component_icons_frag"),
        },
        {
            // Vertex inputs
            RenderPipeline::VertexInput::of<ComponentIcon::Point>(0, VK_VERTEX_INPUT_RATE_INSTANCE),
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
    },
    pipelineCopy{
        vulkan,
        {
            // List of shader modules
            assetsManager.getShaders().find("pass_copy_vert"),
            assetsManager.getShaders().find("pass_copy_frag"),
        },
        {
            // Vertex inputs
            RenderPipeline::VertexInput::of<FullScreenVertex>(0),
        },
        {
            // Additional pipeline options
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            RenderPipeline::DepthMode::Ignore,
            RenderPipeline::Blending::Additive,
            VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_BACK_BIT,
            VK_FRONT_FACE_CLOCKWISE,
        },
    } {

    setAttachments({
        RenderPassNonHdr::Attachments::Depth,
        RenderPassNonHdr::Attachments::Forward,
    });

    addPipeline(pipelineWorldText);
    addPipeline(pipelineIcons);
    addPipeline(pipelineCopy);
}

void RenderSubpassNonHdr::render(VulkanCommandBuffer& vkb, Scene& scene) {
    pipelineWorldText.getDescriptorPool().reset();
    pipelineIcons.getDescriptorPool().reset();
    pipelineCopy.getDescriptorPool().reset();

    std::vector<ForwardRenderJob> jobs;
    collectForRender<ComponentWorldText>(vkb, scene, jobs);

    std::sort(jobs.begin(), jobs.end(), [](auto& a, auto& b) { return a.order > b.order; });

    currentPipeline = nullptr;
    for (auto& job : jobs) {
        job.fn();
    }

    renderSceneIcons(vkb, scene);
    renderSceneOutline(vkb, scene);
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

void RenderSubpassNonHdr::renderSceneIcons(VulkanCommandBuffer& vkb, Scene& scene) {
    auto& camera = *scene.getPrimaryCamera();
    auto& controllerIcons = scene.getController<ControllerIcon>();

    controllerIcons.recalculate(vulkan);

    std::array<const ControllerIcon::Buffers*, 2> buffers{};
    buffers[0] = &controllerIcons.getStaticBuffers();
    buffers[1] = &controllerIcons.getDynamicBuffers();

    if (buffers[0]->empty() && buffers[1]->empty()) {
        return;
    }

    if (currentPipeline != &pipelineIcons) {
        currentPipeline = &pipelineIcons;
        currentPipeline->bind(vkb);
    }

    const auto modelMatrix = Matrix4{1.0f};
    const float scale = camera.isOrthographic() ? 110.0f : 1.0f; // TODO: WTF?
    pipelineIcons.pushConstants(vkb, PushConstant{"modelMatrix", modelMatrix}, PushConstant{"scale", scale});

    for (const auto* b : buffers) {
        for (const auto& pair : *b) {
            if (pair.second.count() == 0) {
                continue;
            }

            std::array<UniformBindingRef, 1> uniforms{};
            uniforms[0] = {"Camera", camera.getUbo().getCurrentBuffer()};

            std::array<SamplerBindingRef, 1> textures{};
            textures[0] = {"colorTexture", *pair.first};

            pipelineIcons.bindDescriptors(vkb, uniforms, textures, {});

            std::array<VulkanVertexBufferBindRef, 1> vboBindings{};
            vboBindings[0] = {&pair.second.getCurrentBuffer(), 0};
            vkb.bindBuffers(vboBindings);

            vkb.draw(4, pair.second.count(), 0, 0);
        }
    }
}

void RenderSubpassNonHdr::renderSceneOutline(VulkanCommandBuffer& vkb, Scene& scene) {
    pipelineCopy.bind(vkb);

    std::array<SamplerBindingRef, 1> textures{};
    textures[0] = {"texColor", outlineTexture};

    pipelineCopy.bindDescriptors(vkb, {}, textures, {});

    pipelineCopy.renderMesh(vkb, resources.getMeshFullScreenQuad());
}

/*void RenderSubpassNonHdr::renderSceneForward(VulkanCommandBuffer& vkb, const ComponentCamera& camera,
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
}*/
