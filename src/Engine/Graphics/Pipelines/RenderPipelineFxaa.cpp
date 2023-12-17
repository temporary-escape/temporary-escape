#include "RenderPipelineFxaa.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../MeshUtils.hpp"
#include <pass_fxaa_frag.spirv.h>
#include <pass_fxaa_vert.spirv.h>

using namespace Engine;

RenderPipelineFXAA::RenderPipelineFXAA(VulkanRenderer& vulkan) : RenderPipeline{vulkan, "RenderPipelineFXAA"} {

    addShader(Embed::pass_fxaa_vert_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
    addShader(Embed::pass_fxaa_frag_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
    addVertexInput(RenderPipeline::VertexInput::of<FullScreenVertex>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::Ignore);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_CLOCKWISE);
    setBlending(Blending::None);
}

void RenderPipelineFXAA::setTextureSize(const Vector2& value) {
    pushConstants(PushConstant{"textureSize", value});
}

void RenderPipelineFXAA::setTexture(const VulkanTexture& texture) {
    textures[0] = {"texColor", texture};
}

void RenderPipelineFXAA::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, {}, textures, {});
}
