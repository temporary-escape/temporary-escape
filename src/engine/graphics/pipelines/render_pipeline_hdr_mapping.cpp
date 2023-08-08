#include "render_pipeline_hdr_mapping.hpp"
#include "../../assets/assets_manager.hpp"
#include "../mesh_utils.hpp"

using namespace Engine;

RenderPipelineHDRMapping::RenderPipelineHDRMapping(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelineHDRMapping"} {

    addShader(assetsManager.getShaders().find("pass_hdr_mapping_vert"));
    addShader(assetsManager.getShaders().find("pass_hdr_mapping_frag"));
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
