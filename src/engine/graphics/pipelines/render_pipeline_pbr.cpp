#include "render_pipeline_pbr.hpp"
#include "../../assets/assets_manager.hpp"
#include "../mesh_utils.hpp"

using namespace Engine;

RenderPipelinePbr::RenderPipelinePbr(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelinePbr"} {

    addShader(assetsManager.getShaders().find("pass_pbr_vert"));
    addShader(assetsManager.getShaders().find("pass_pbr_frag"));
    addVertexInput(RenderPipeline::VertexInput::of<FullScreenVertex>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::Ignore);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_CLOCKWISE);
    setBlending(Blending::None);
}

void RenderPipelinePbr::setUniformCamera(const VulkanBuffer& ubo) {
    uniforms[0] = {"Camera", ubo};
}

void RenderPipelinePbr::setUniformDirectionalLights(const VulkanBuffer& ubo) {
    uniforms[1] = {"DirectionalLights", ubo};
}

void RenderPipelinePbr::setUniformShadowsViewProj(const VulkanBuffer& ubo) {
    uniforms[2] = {"ShadowsViewProj", ubo};
}

void RenderPipelinePbr::setTextureIrradiance(const VulkanTexture& texture) {
    textures[0] = {"texIrradiance", texture};
}

void RenderPipelinePbr::setTexturePrefilter(const VulkanTexture& texture) {
    textures[1] = {"texPrefilter", texture};
}

void RenderPipelinePbr::setTextureBrdf(const VulkanTexture& texture) {
    textures[2] = {"texBrdf", texture};
}

void RenderPipelinePbr::setTextureDepth(const VulkanTexture& texture) {
    textures[3] = {"texDepth", texture};
}

void RenderPipelinePbr::setTextureBaseColorAmbient(const VulkanTexture& texture) {
    textures[4] = {"texBaseColorAmbient", texture};
}

void RenderPipelinePbr::setTextureEmissiveRoughness(const VulkanTexture& texture) {
    textures[5] = {"texEmissiveRoughness", texture};
}

void RenderPipelinePbr::setTextureNormalMetallic(const VulkanTexture& texture) {
    textures[6] = {"texNormalMetallic", texture};
}

void RenderPipelinePbr::setTextureSSAO(const VulkanTexture& texture) {
    textures[7] = {"texSsao", texture};
}

void RenderPipelinePbr::setTextureShadows(const VulkanTexture& texture) {
    textures[8] = {"texShadows", texture};
}

void RenderPipelinePbr::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, textures, {});
}
