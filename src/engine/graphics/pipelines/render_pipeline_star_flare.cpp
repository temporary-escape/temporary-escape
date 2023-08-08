#include "render_pipeline_star_flare.hpp"
#include "../../assets/assets_manager.hpp"
#include "../mesh_utils.hpp"

using namespace Engine;

RenderPipelineStarFlare::RenderPipelineStarFlare(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelineStarFlare"} {

    addShader(assetsManager.getShaders().find("component_star_flare_vert"));
    addShader(assetsManager.getShaders().find("component_star_flare_frag"));
    addVertexInput(RenderPipeline::VertexInput::of<FullScreenVertex>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::Ignore);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_CLOCKWISE);
    setBlending(Blending::None);
    setStencil(Stencil::Read, 0x00);
}

void RenderPipelineStarFlare::setModelMatrix(const Matrix4& value) {
    pushConstants(PushConstant{"modelMatrix", value});
}

void RenderPipelineStarFlare::setSize(const Vector2& value) {
    pushConstants(PushConstant{"size", value});
}

void RenderPipelineStarFlare::setTemp(float value) {
    pushConstants(PushConstant{"temp", value});
}

void RenderPipelineStarFlare::setUniformCamera(const VulkanBuffer& ubo) {
    uniforms[0] = {"Camera", ubo};
}

void RenderPipelineStarFlare::setTextureColor(const VulkanTexture& texture) {
    textures[0] = {"colorTexture", texture};
}

void RenderPipelineStarFlare::setTextureSpectrumLow(const VulkanTexture& texture) {
    textures[1] = {"spectrumLowTexture", texture};
}

void RenderPipelineStarFlare::setTextureSpectrumHigh(const VulkanTexture& texture) {
    textures[2] = {"spectrumHighTexture", texture};
}

void RenderPipelineStarFlare::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, textures, {});
}
