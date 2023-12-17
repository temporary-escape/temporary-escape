#include "RenderPipelineBloomUpsample.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../MeshUtils.hpp"
#include <pass_bloom_upsample_frag.spirv.h>
#include <pass_bloom_upsample_vert.spirv.h>

using namespace Engine;

RenderPipelineBloomUpsample::RenderPipelineBloomUpsample(VulkanRenderer& vulkan) :
    RenderPipeline{vulkan, "RenderPipelineBloomUpsample"} {

    addShader(Embed::pass_bloom_upsample_vert_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
    addShader(Embed::pass_bloom_upsample_frag_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
    addVertexInput(RenderPipeline::VertexInput::of<FullScreenVertex>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::Ignore);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_CLOCKWISE);
    setBlending(Blending::None);
}

void RenderPipelineBloomUpsample::setFilterRadius(const float value) {
    pushConstants(PushConstant{"filterRadius", value});
}

void RenderPipelineBloomUpsample::setTexture(const VulkanTexture& texture) {
    textures[0] = {"texColor", texture};
}

void RenderPipelineBloomUpsample::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, {}, textures, {});
}
