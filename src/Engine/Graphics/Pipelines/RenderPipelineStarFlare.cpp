#include "RenderPipelineStarFlare.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../MeshUtils.hpp"
#include <component_star_flare_frag.spirv.h>
#include <component_star_flare_vert.spirv.h>

using namespace Engine;

RenderPipelineStarFlare::RenderPipelineStarFlare(VulkanRenderer& vulkan) :
    RenderPipeline{vulkan, "RenderPipelineStarFlare"} {

    addShader(Embed::component_star_flare_vert_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
    addShader(Embed::component_star_flare_frag_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
    addVertexInput(RenderPipeline::VertexInput::of<FullScreenVertex>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
    setDepthMode(DepthMode::Read);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::Additive);
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
