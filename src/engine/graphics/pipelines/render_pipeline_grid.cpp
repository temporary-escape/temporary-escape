#include "render_pipeline_grid.hpp"
#include "../../assets/assets_manager.hpp"
#include "../mesh_utils.hpp"

using namespace Engine;

RenderPipelineGrid::RenderPipelineGrid(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelineGrid"} {

    addShader(assetsManager.getShaders().find("component_grid_vert"));
    addShader(assetsManager.getShaders().find("component_grid_frag"));
    addVertexInput(RenderPipeline::VertexInput::of<VoxelShape::VertexFinal>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::ReadWrite);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::None);
}

void RenderPipelineGrid::setModelMatrix(const Matrix4& value) {
    pushConstants(PushConstant{"modelMatrix", value});
}

void RenderPipelineGrid::setNormalMatrix(const Matrix3& value) {
    pushConstants(PushConstant{"normalMatrix", value});
}

void RenderPipelineGrid::setEntityColor(const Color4& value) {
    pushConstants(PushConstant{"entityColor", value});
}

void RenderPipelineGrid::setUniformCamera(const VulkanBuffer& ubo) {
    uniforms[0] = {"Camera", ubo};
}

void RenderPipelineGrid::setUniformMaterial(const VulkanBuffer& ubo) {
    uniforms[1] = {"Material", ubo};
}

void RenderPipelineGrid::setTextureBaseColor(const VulkanTexture& texture) {
    textures[0] = {"baseColorTexture", texture};
}

void RenderPipelineGrid::setTextureEmissive(const VulkanTexture& texture) {
    textures[1] = {"emissiveTexture", texture};
}

void RenderPipelineGrid::setTextureMetallicRoughness(const VulkanTexture& texture) {
    textures[4] = {"metallicRoughnessTexture", texture};
}

void RenderPipelineGrid::setTextureNormal(const VulkanTexture& texture) {
    textures[2] = {"normalTexture", texture};
}

void RenderPipelineGrid::setTexturePalette(const VulkanTexture& texture) {
    textures[6] = {"paletteTexture", texture};
}

void RenderPipelineGrid::setTextureMask(const VulkanTexture& texture) {
    textures[5] = {"maskTexture", texture};
}

void RenderPipelineGrid::setTextureAmbientOcclusion(const VulkanTexture& texture) {
    textures[3] = {"ambientOcclusionTexture", texture};
}

void RenderPipelineGrid::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, textures, {});
}
