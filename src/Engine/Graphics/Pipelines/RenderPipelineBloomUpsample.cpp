#include "RenderPipelineBloomUpsample.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../MeshUtils.hpp"

using namespace Engine;

RenderPipelineBloomUpsample::RenderPipelineBloomUpsample(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelineBloomUpsample"} {

    addShader(assetsManager.getShaders().find("pass_bloom_upsample_vert"));
    addShader(assetsManager.getShaders().find("pass_bloom_upsample_frag"));
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
