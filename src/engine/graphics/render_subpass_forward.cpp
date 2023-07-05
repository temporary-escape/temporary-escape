#include "render_subpass_forward.hpp"
#include "../assets/assets_manager.hpp"
#include "render_pass_forward.hpp"
#include "skybox.hpp"

using namespace Engine;

RenderSubpassForward::RenderSubpassForward(VulkanRenderer& vulkan, RenderResources& resources,
                                           AssetsManager& assetsManager) :
    vulkan{vulkan},
    resources{resources},
    pipelineDebug{
        vulkan,
        {
            // List of shader modules
            assetsManager.getShaders().find("component_debug_vert"),
            assetsManager.getShaders().find("component_debug_frag"),
        },
        {
            // Vertex inputs
            RenderPipeline::VertexInput::of<ComponentDebug::Vertex>(0),
        },
        {
            // Additional pipeline options
            VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
            RenderPipeline::DepthMode::ReadWrite,
            RenderPipeline::Blending::Normal,
            VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_BACK_BIT,
            VK_FRONT_FACE_COUNTER_CLOCKWISE,
        },
    },
    pipelineLines{
        vulkan,
        {
            // List of shader modules
            assetsManager.getShaders().find("component_lines_vert"),
            assetsManager.getShaders().find("component_lines_frag"),
        },
        {
            // Vertex inputs
            RenderPipeline::VertexInput::of<ComponentLines::Vertex>(0),
        },
        {
            // Additional pipeline options
            VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
            RenderPipeline::DepthMode::ReadWrite,
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
            RenderPipeline::DepthMode::Read,
            RenderPipeline::Blending::Normal,
            VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_BACK_BIT,
            VK_FRONT_FACE_COUNTER_CLOCKWISE,
        },
    },
    pipelinePolyShape{
        vulkan,
        {
            // List of shader modules
            assetsManager.getShaders().find("component_poly_shape_vert"),
            assetsManager.getShaders().find("component_poly_shape_frag"),
        },
        {
            // Vertex inputs
            RenderPipeline::VertexInput::of<ComponentPolyShape::Point>(0),
        },
        {
            // Additional pipeline options
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            RenderPipeline::DepthMode::Read,
            RenderPipeline::Blending::Normal,
            VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_BACK_BIT,
            VK_FRONT_FACE_CLOCKWISE,
        },
    },
    pipelineStarFlare{
        vulkan,
        {
            // List of shader modules
            assetsManager.getShaders().find("component_star_flare_vert"),
            assetsManager.getShaders().find("component_star_flare_frag"),
        },
        {
            // Vertex inputs
        },
        {
            // Additional pipeline options
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
            RenderPipeline::DepthMode::Read,
            RenderPipeline::Blending::Additive,
            VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_BACK_BIT,
            VK_FRONT_FACE_COUNTER_CLOCKWISE,
            RenderPipeline::Stencil::Read,
            0x00,
        },
    } {

    setAttachments({
        RenderPassForward::Attachments::Depth,
        RenderPassForward::Attachments::Forward,
    });

    addPipeline(pipelineDebug);
    addPipeline(pipelineLines);
    addPipeline(pipelinePointCloud);
    addPipeline(pipelinePolyShape);
    addPipeline(pipelineStarFlare);
}

void RenderSubpassForward::render(VulkanCommandBuffer& vkb, Scene& scene) {
    pipelineDebug.getDescriptorPool().reset();
    pipelineLines.getDescriptorPool().reset();
    pipelinePointCloud.getDescriptorPool().reset();
    pipelinePolyShape.getDescriptorPool().reset();
    pipelineStarFlare.getDescriptorPool().reset();

    std::vector<ForwardRenderJob> jobs;
    collectForRender<ComponentDebug>(vkb, scene, jobs);
    collectForRender<ComponentLines>(vkb, scene, jobs);
    collectForRender<ComponentPointCloud>(vkb, scene, jobs);
    collectForRender<ComponentPolyShape>(vkb, scene, jobs);
    collectForRender<ComponentStarFlare>(vkb, scene, jobs);

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
        pipelineDebug.bind(vkb);
    }

    std::array<UniformBindingRef, 1> uniforms{};
    uniforms[0] = {"Camera", camera.getUbo().getCurrentBuffer()};

    pipelineDebug.bindDescriptors(vkb, uniforms, {}, {});

    const auto modelMatrix = transform.getAbsoluteTransform();
    pipelineDebug.pushConstants(vkb, PushConstant{"modelMatrix", modelMatrix});

    pipelineDebug.renderMesh(vkb, mesh);
}

void RenderSubpassForward::renderSceneForward(VulkanCommandBuffer& vkb, const ComponentCamera& camera,
                                              ComponentTransform& transform, ComponentLines& component) {
    component.recalculate(vulkan);

    const auto& mesh = component.getMesh();
    if (mesh.count == 0) {
        return;
    }

    if (currentPipeline != &pipelineLines) {
        currentPipeline = &pipelineLines;
        pipelineLines.bind(vkb);
    }

    std::array<UniformBindingRef, 1> uniforms{};
    uniforms[0] = {"Camera", camera.getUbo().getCurrentBuffer()};

    pipelineLines.bindDescriptors(vkb, uniforms, {}, {});

    const auto modelMatrix = transform.getAbsoluteTransform();
    const auto color = component.getColor();
    pipelineLines.pushConstants(vkb, PushConstant{"modelMatrix", modelMatrix}, PushConstant{"color", color});

    pipelineLines.renderMesh(vkb, mesh);
}

void RenderSubpassForward::renderSceneForward(VulkanCommandBuffer& vkb, const ComponentCamera& camera,
                                              ComponentTransform& transform, ComponentPointCloud& component) {
    component.recalculate(vulkan);

    const auto& mesh = component.getMesh();
    if (mesh.count == 0) {
        return;
    }

    if (currentPipeline != &pipelinePointCloud) {
        currentPipeline = &pipelinePointCloud;
        pipelinePointCloud.bind(vkb);
    }

    std::array<UniformBindingRef, 1> uniforms{};
    uniforms[0] = {"Camera", camera.getUbo().getCurrentBuffer()};

    std::array<SamplerBindingRef, 1> textures{};
    textures[0] = {"colorTexture", component.getTexture()->getVulkanTexture()};

    pipelinePointCloud.bindDescriptors(vkb, uniforms, textures, {});

    const auto modelMatrix = transform.getAbsoluteTransform();
    pipelinePointCloud.pushConstants(vkb, PushConstant{"modelMatrix", modelMatrix});

    pipelinePointCloud.renderMesh(vkb, mesh);
}

void RenderSubpassForward::renderSceneForward(VulkanCommandBuffer& vkb, const ComponentCamera& camera,
                                              ComponentTransform& transform, ComponentPolyShape& component) {
    component.recalculate(vulkan);

    const auto& mesh = component.getMesh();
    if (mesh.count == 0) {
        return;
    }

    if (currentPipeline != &pipelinePolyShape) {
        currentPipeline = &pipelinePolyShape;
        pipelinePolyShape.bind(vkb);
    }

    std::array<UniformBindingRef, 1> uniforms{};
    uniforms[0] = {"Camera", camera.getUbo().getCurrentBuffer()};

    pipelinePolyShape.bindDescriptors(vkb, uniforms, {}, {});

    const auto modelMatrix = transform.getAbsoluteTransform();
    pipelinePolyShape.pushConstants(vkb, PushConstant{"modelMatrix", modelMatrix});

    pipelinePolyShape.renderMesh(vkb, mesh);
}

void RenderSubpassForward::renderSceneForward(VulkanCommandBuffer& vkb, const ComponentCamera& camera,
                                              ComponentTransform& transform, ComponentStarFlare& component) {
    const auto& mesh = component.getMesh();
    if (mesh.count == 0) {
        return;
    }

    if (currentPipeline != &pipelineStarFlare) {
        currentPipeline = &pipelineStarFlare;
        pipelineStarFlare.bind(vkb);
    }

    std::array<UniformBindingRef, 1> uniforms{};
    if (component.isBackground()) {
        uniforms[0] = {"Camera", camera.getUboZeroPos().getCurrentBuffer()};
    } else {
        uniforms[0] = {"Camera", camera.getUbo().getCurrentBuffer()};
    }

    std::array<SamplerBindingRef, 3> textures{};
    textures[0] = {"colorTexture", component.getTexture()->getVulkanTexture()};
    textures[1] = {"spectrumLowTexture", component.getTextureLow()->getVulkanTexture()};
    textures[2] = {"spectrumHighTexture", component.getTextureHigh()->getVulkanTexture()};

    pipelineStarFlare.bindDescriptors(vkb, uniforms, textures, {});

    const auto modelMatrix = transform.getAbsoluteTransform();
    pipelineStarFlare.pushConstants(vkb,
                                    PushConstant{"modelMatrix", modelMatrix},
                                    PushConstant{"size", component.getSize()},
                                    PushConstant{"temp", component.getTemperature()});

    pipelineStarFlare.renderMesh(vkb, mesh);
}
