#include "render_pipeline_fxaa.hpp"
#include "../../assets/assets_manager.hpp"
#include "../mesh_utils.hpp"

using namespace Engine;

RenderPipelineFXAA::RenderPipelineFXAA(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelineFXAA"} {

    addShader(assetsManager.getShaders().find("pass_fxaa_vert"));
    addShader(assetsManager.getShaders().find("pass_fxaa_frag"));
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
