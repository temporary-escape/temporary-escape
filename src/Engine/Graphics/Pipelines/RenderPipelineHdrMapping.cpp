#include "RenderPipelineHdrMapping.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../MeshUtils.hpp"
#include <pass_hdr_mapping_frag.spirv.h>
#include <pass_hdr_mapping_vert.spirv.h>

using namespace Engine;

RenderPipelineHDRMapping::RenderPipelineHDRMapping(VulkanRenderer& vulkan) :
    RenderPipeline{vulkan, "RenderPipelineHDRMapping"} {

    addShader(Embed::pass_hdr_mapping_vert_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
    addShader(Embed::pass_hdr_mapping_frag_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
    addVertexInput(RenderPipeline::VertexInput::of<FullScreenVertex>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::Ignore);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_CLOCKWISE);
    setBlending(Blending::None);
}

void RenderPipelineHDRMapping::setBloomStrength(float value) {
    pushConstants(PushConstant{"bloomStrength", value});
}

void RenderPipelineHDRMapping::setGamma(float value) {
    pushConstants(PushConstant{"gamma", value});
}

void RenderPipelineHDRMapping::setExposure(float value) {
    pushConstants(PushConstant{"exposure", value});
}

void RenderPipelineHDRMapping::setTextureColor(const VulkanTexture& texture) {
    textures[0] = {"texColor", texture};
}

void RenderPipelineHDRMapping::setTextureBloom(const VulkanTexture& texture) {
    textures[1] = {"texBloom", texture};
}

void RenderPipelineHDRMapping::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, {}, textures, {});
}
