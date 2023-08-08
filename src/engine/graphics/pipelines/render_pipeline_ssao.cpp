#include "render_pipeline_ssao.hpp"
#include "../../assets/assets_manager.hpp"
#include "../mesh_utils.hpp"

using namespace Engine;

RenderPipelineSSAO::RenderPipelineSSAO(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelineSSAO"} {

    addShader(assetsManager.getShaders().find("pass_ssao_vert"));
    addShader(assetsManager.getShaders().find("pass_ssao_frag"));
    addVertexInput(RenderPipeline::VertexInput::of<FullScreenVertex>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::Ignore);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_CLOCKWISE);
    setBlending(Blending::None);
}

void RenderPipelineSSAO::setKernelSize(const int value) {
    pushConstants(PushConstant{"kernelSize", value});
}

void RenderPipelineSSAO::setScale(const Vector2& value) {
    pushConstants(PushConstant{"scale", value});
}

void RenderPipelineSSAO::setUniformCamera(const VulkanBuffer& ubo) {
    uniforms[0] = {"Camera", ubo};
}

void RenderPipelineSSAO::setUniformSamples(const VulkanBuffer& ubo) {
    uniforms[1] = {"Samples", ubo};
}

void RenderPipelineSSAO::setTextureNoise(const VulkanTexture& texture) {
    textures[0] = {"noiseTexture", texture};
}

void RenderPipelineSSAO::setTextureDepth(const VulkanTexture& texture) {
    textures[1] = {"depthTexture", texture};
}

void RenderPipelineSSAO::setTextureNormal(const VulkanTexture& texture) {
    textures[2] = {"normalTexture", texture};
}

void RenderPipelineSSAO::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, textures, {});
}
