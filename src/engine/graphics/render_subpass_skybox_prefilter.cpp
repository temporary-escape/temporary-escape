#include "render_subpass_skybox_prefilter.hpp"
#include "../assets/assets_manager.hpp"
#include "mesh_utils.hpp"
#include "render_pass_skybox_prefilter.hpp"

using namespace Engine;

RenderSubpassSkyboxPrefilter::RenderSubpassSkyboxPrefilter(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    vulkan{vulkan},
    pipelinePrefilter{
        vulkan,
        {
            // List of shader modules
            assetsManager.getShaders().find("skybox_prefilter_vert"),
            assetsManager.getShaders().find("skybox_prefilter_frag"),
        },
        {
            // Vertex inputs
            RenderPipeline::VertexInput::of<SkyboxVertex>(0),
        },
        {
            // Additional pipeline options
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            RenderPipeline::DepthMode::Ignore,
            RenderPipeline::Blending::None,
            VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_BACK_BIT,
            VK_FRONT_FACE_COUNTER_CLOCKWISE,
        },
    } {

    setAttachments({
        RenderPassSkyboxPrefilter::Attachments::Prefilter,
    });

    addPipeline(pipelinePrefilter);

    skyboxMesh = createSkyboxCube(vulkan);
}

void RenderSubpassSkyboxPrefilter::reset() {
    pipelinePrefilter.getDescriptorPool().reset();
}

void RenderSubpassSkyboxPrefilter::render(VulkanCommandBuffer& vkb, const VulkanTexture& skyboxColor,
                                          const Matrix4& projection, const Matrix4& view, const float roughness) {
    pipelinePrefilter.bind(vkb);

    std::array<SamplerBindingRef, 1> textures{};
    textures[0] = {"texSkybox", skyboxColor};

    pipelinePrefilter.bindDescriptors(vkb, {}, textures, {});

    const auto projectionViewMatrix = projection * view;
    pipelinePrefilter.pushConstants(
        vkb, PushConstant{"projectionViewMatrix", projectionViewMatrix}, PushConstant{"roughness", roughness});

    pipelinePrefilter.renderMesh(vkb, skyboxMesh);
}
