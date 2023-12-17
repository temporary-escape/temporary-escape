#include "RenderPipelineBloomDownsample.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../MeshUtils.hpp"
#include <pass_bloom_downsample_frag.spirv.h>
#include <pass_bloom_downsample_vert.spirv.h>

using namespace Engine;

RenderPipelineBloomDownsample::RenderPipelineBloomDownsample(VulkanRenderer& vulkan) :
    RenderPipeline{vulkan, "RenderPipelineBloomDownsample"} {

    addShader(Embed::pass_bloom_downsample_vert_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
    addShader(Embed::pass_bloom_downsample_frag_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
    addVertexInput(RenderPipeline::VertexInput::of<FullScreenVertex>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::Ignore);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_CLOCKWISE);
    setBlending(Blending::None);
}

void RenderPipelineBloomDownsample::setTextureSize(const Vector2& value) {
    pushConstants(PushConstant{"texColorSize", value});
}

void RenderPipelineBloomDownsample::setMipLevel(const int value) {
    pushConstants(PushConstant{"mipLevel", value});
}

void RenderPipelineBloomDownsample::setTexture(const VulkanTexture& texture) {
    textures[0] = {"texColor", texture};
}

void RenderPipelineBloomDownsample::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, {}, textures, {});
}
