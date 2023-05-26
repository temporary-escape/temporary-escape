#include "render_subpass_skybox_irradiance.hpp"
#include "../assets/assets_manager.hpp"
#include "mesh_utils.hpp"
#include "render_pass_skybox_irradiance.hpp"

using namespace Engine;

RenderSubpassSkyboxIrradiance::RenderSubpassSkyboxIrradiance(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    vulkan{vulkan},
    pipelineIrradiance{
        vulkan,
        {
            // List of shader modules
            assetsManager.getShaders().find("skybox_irradiance_vert"),
            assetsManager.getShaders().find("skybox_irradiance_frag"),
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
        RenderPassSkyboxIrradiance::Attachments::Irradiance,
    });

    addPipeline(pipelineIrradiance);

    skyboxMesh = createSkyboxCube(vulkan);
}

void RenderSubpassSkyboxIrradiance::reset() {
    pipelineIrradiance.getDescriptorPool().reset();
}

void RenderSubpassSkyboxIrradiance::render(VulkanCommandBuffer& vkb, const VulkanTexture& skyboxColor,
                                           const Matrix4& projection, const Matrix4& view) {
    pipelineIrradiance.bind(vkb);

    std::array<SamplerBindingRef, 1> textures{};
    textures[0] = {"texSkybox", skyboxColor};

    pipelineIrradiance.bindDescriptors(vkb, {}, textures, {});

    const auto projectionViewMatrix = projection * view;
    pipelineIrradiance.pushConstants(vkb, PushConstant{"projectionViewMatrix", projectionViewMatrix});

    pipelineIrradiance.renderMesh(vkb, skyboxMesh);
}
