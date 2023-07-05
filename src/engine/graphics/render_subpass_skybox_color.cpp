#include "render_subpass_skybox_color.hpp"
#include "../assets/assets_manager.hpp"
#include "../math/random.hpp"
#include "mesh_utils.hpp"
#include "render_pass_skybox_color.hpp"

using namespace Engine;

RenderSubpassSkyboxColor::RenderSubpassSkyboxColor(VulkanRenderer& vulkan, RenderResources& resources,
                                                   AssetsManager& assetsManager) :
    vulkan{vulkan},
    resources{resources},
    pipelineNebula{
        vulkan,
        {
            // List of shader modules
            assetsManager.getShaders().find("skybox_nebula_vert"),
            assetsManager.getShaders().find("skybox_nebula_frag"),
        },
        {
            // Vertex inputs
            RenderPipeline::VertexInput::of<SkyboxVertex>(0),
        },
        {
            // Additional pipeline options
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            RenderPipeline::DepthMode::Ignore,
            RenderPipeline::Blending::Additive,
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
            RenderPipeline::Blending::Additive,
            VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_BACK_BIT,
            VK_FRONT_FACE_COUNTER_CLOCKWISE,
        },
    } {

    setAttachments({
        RenderPassSkyboxColor::Attachments::Color,
    });

    addPipeline(pipelineNebula);
    addPipeline(pipelinePointCloud);
}

void RenderSubpassSkyboxColor::render(VulkanCommandBuffer& vkb, Scene& scene) {
    pipelineNebula.getDescriptorPool().reset();
    pipelinePointCloud.getDescriptorPool().reset();

    renderNebulas(vkb, scene);
    renderPointClouds(vkb, scene);
}

void RenderSubpassSkyboxColor::renderNebulas(VulkanCommandBuffer& vkb, Scene& scene) {
    auto systemNebulas = scene.getView<ComponentNebula>();
    auto camera = scene.getPrimaryCamera();

    pipelineNebula.bind(vkb);

    std::array<UniformBindingRef, 2> uniforms{};
    uniforms[0] = {"Camera", camera->getUbo().getCurrentBuffer()};

    for (auto&& [entity, nebula] : systemNebulas.each()) {
        nebula.recalculate(vulkan);

        uniforms[1] = {"Nebula", nebula.getUbo()};

        pipelineNebula.bindDescriptors(vkb, uniforms, {}, {});

        pipelineNebula.renderMesh(vkb, resources.getMeshSkyboxCube());
    }
}

void RenderSubpassSkyboxColor::renderPointClouds(VulkanCommandBuffer& vkb, Scene& scene) {
    auto systemPointClouds = scene.getView<ComponentTransform, ComponentPointCloud>();
    auto camera = scene.getPrimaryCamera();

    pipelinePointCloud.bind(vkb);

    std::array<UniformBindingRef, 1> uniforms{};
    uniforms[0] = {"Camera", camera->getUbo().getCurrentBuffer()};

    std::array<SamplerBindingRef, 1> textures{};

    for (auto&& [entity, transform, pointCloud] : systemPointClouds.each()) {
        pointCloud.recalculate(vulkan);

        const auto& mesh = pointCloud.getMesh();
        if (mesh.count == 0) {
            continue;
        }

        textures[0] = {"colorTexture", pointCloud.getTexture()->getVulkanTexture()};

        pipelinePointCloud.bindDescriptors(vkb, uniforms, textures, {});

        const auto modelMatrix = transform.getAbsoluteTransform();
        pipelinePointCloud.pushConstants(vkb, PushConstant{"modelMatrix", modelMatrix});

        pipelinePointCloud.renderMesh(vkb, mesh);
    }
}
