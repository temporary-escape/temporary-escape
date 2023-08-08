#include "render_pipeline_skybox_irradiance.hpp"
#include "../../assets/assets_manager.hpp"
#include "../mesh_utils.hpp"

using namespace Engine;

RenderPipelineSkyboxIrradiance::RenderPipelineSkyboxIrradiance(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelineSkyboxIrradiance"} {

    addShader(assetsManager.getShaders().find("skybox_irradiance_vert"));
    addShader(assetsManager.getShaders().find("skybox_irradiance_frag"));
    addVertexInput(RenderPipeline::VertexInput::of<SkyboxVertex>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::Ignore);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::None);
}

void RenderPipelineSkyboxIrradiance::setProjectionViewMatrix(const Matrix4& value) {
    pushConstants(PushConstant{"projectionViewMatrix", value});
}

void RenderPipelineSkyboxIrradiance::setTextureSkybox(const VulkanTexture& texture) {
    textures[0] = {"texSkybox", texture};
}

void RenderPipelineSkyboxIrradiance::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, {}, textures, {});
}
