#include "RenderPipelineBlit.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../MeshUtils.hpp"

using namespace Engine;

RenderPipelineBlit::RenderPipelineBlit(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelineBlit"} {

    addShader(assetsManager.getShaders().find("blit_vert"));
    addShader(assetsManager.getShaders().find("blit_frag"));
    addVertexInput(RenderPipeline::VertexInput::of<FullScreenVertex>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::Ignore);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_CLOCKWISE);
    setBlending(Blending::None);
}

void RenderPipelineBlit::setTexture(const VulkanTexture& texture) {
    textures[0] = {"colorTexture", texture};
}

void RenderPipelineBlit::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, {}, textures, {});
}
