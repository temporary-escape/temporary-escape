#include "render_pipeline_skybox_prefilter.hpp"
#include "../../assets/assets_manager.hpp"
#include "../mesh_utils.hpp"

using namespace Engine;

RenderPipelineSkyboxPrefilter::RenderPipelineSkyboxPrefilter(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelineSkyboxIrradiance"} {

    addShader(assetsManager.getShaders().find("skybox_prefilter_vert"));
    addShader(assetsManager.getShaders().find("skybox_prefilter_frag"));
    addVertexInput(RenderPipeline::VertexInput::of<SkyboxVertex>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::Ignore);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::None);
}

void RenderPipelineSkyboxPrefilter::setProjectionViewMatrix(const Matrix4& value) {
    pushConstants(PushConstant{"projectionViewMatrix", value});
}

void RenderPipelineSkyboxPrefilter::setRoughness(const float value) {
    pushConstants(PushConstant{"roughness", value});
}

void RenderPipelineSkyboxPrefilter::setTextureSkybox(const VulkanTexture& texture) {
    textures[0] = {"texSkybox", texture};
}

void RenderPipelineSkyboxPrefilter::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, {}, textures, {});
}
