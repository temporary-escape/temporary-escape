#include "RenderPipelineModelSkinned.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Components/ComponentModelSkinned.hpp"
#include <component_model_frag.spirv.h>
#include <component_model_skinned_vert.spirv.h>

using namespace Engine;

RenderPipelineModelSkinned::RenderPipelineModelSkinned(VulkanRenderer& vulkan) :
    RenderPipeline{vulkan, "RenderPipelineModelSkinned"} {

    addShader(Embed::component_model_skinned_vert_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
    addShader(Embed::component_model_frag_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
    addVertexInput(RenderPipeline::VertexInput::of<ComponentModelSkinned::Vertex>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::ReadWrite);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::None);
}

void RenderPipelineModelSkinned::setModelMatrix(const Matrix4& value) {
    pushConstants(PushConstant{"modelMatrix", value});
}

void RenderPipelineModelSkinned::setNormalMatrix(const Matrix3& value) {
    pushConstants(PushConstant{"normalMatrix", value});
}

void RenderPipelineModelSkinned::setEntityColor(const Color4& value) {
    pushConstants(PushConstant{"entityColor", value});
}

void RenderPipelineModelSkinned::setUniformCamera(const VulkanBuffer& ubo) {
    uniforms[0] = {"Camera", ubo};
}

void RenderPipelineModelSkinned::setUniformArmature(const VulkanBuffer& ubo, const size_t offset) {
    uniforms[1] = {"Armature", ubo, offset, sizeof(ComponentModelSkinned::Armature)};
}

void RenderPipelineModelSkinned::setUniformMaterial(const VulkanBuffer& ubo) {
    uniforms[2] = {"Material", ubo};
}

void RenderPipelineModelSkinned::setTextureBaseColor(const VulkanTexture& texture) {
    textures[0] = {"baseColorTexture", texture};
}

void RenderPipelineModelSkinned::setTextureEmissive(const VulkanTexture& texture) {
    textures[1] = {"emissiveTexture", texture};
}

void RenderPipelineModelSkinned::setTextureMetallicRoughness(const VulkanTexture& texture) {
    textures[2] = {"metallicRoughnessTexture", texture};
}

void RenderPipelineModelSkinned::setTextureNormal(const VulkanTexture& texture) {
    textures[3] = {"normalTexture", texture};
}

void RenderPipelineModelSkinned::setTextureAmbientOcclusion(const VulkanTexture& texture) {
    textures[4] = {"ambientOcclusionTexture", texture};
}

void RenderPipelineModelSkinned::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, textures, {});
}
