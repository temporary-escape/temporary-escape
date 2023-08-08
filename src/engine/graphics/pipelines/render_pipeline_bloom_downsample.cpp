#include "render_pipeline_bloom_downsample.hpp"
#include "../../assets/assets_manager.hpp"
#include "../mesh_utils.hpp"

using namespace Engine;

RenderPipelineBloomDownsample::RenderPipelineBloomDownsample(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelineBloomDownsample"} {

    addShader(assetsManager.getShaders().find("pass_bloom_downsample_vert"));
    addShader(assetsManager.getShaders().find("pass_bloom_downsample_frag"));
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
