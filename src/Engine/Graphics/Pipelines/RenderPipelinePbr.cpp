#include "RenderPipelinePbr.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../MeshUtils.hpp"
#include <pass_pbr_frag.spirv.h>
#include <pass_pbr_vert.spirv.h>

using namespace Engine;

RenderPipelinePbr::RenderPipelinePbr(VulkanRenderer& vulkan) : RenderPipeline{vulkan, "RenderPipelinePbr"} {

    addShader(Embed::pass_pbr_vert_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
    addShader(Embed::pass_pbr_frag_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
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

void RenderPipelinePbr::setTexturePosition(const VulkanTexture& texture) {
    textures[9] = {"texPosition", texture};
}

void RenderPipelinePbr::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, textures, {});
}
