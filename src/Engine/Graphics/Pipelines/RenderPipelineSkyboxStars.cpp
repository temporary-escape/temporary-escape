#include "RenderPipelineSkyboxStars.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Components/ComponentPointCloud.hpp"
#include "../MeshUtils.hpp"

using namespace Engine;

RenderPipelineSkyboxStars::RenderPipelineSkyboxStars(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelineSkyboxStars"} {

    addShader(assetsManager.getShaders().find("component_point_cloud_vert"));
    addShader(assetsManager.getShaders().find("component_point_cloud_frag"));
    addVertexInput(RenderPipeline::VertexInput::of<ComponentPointCloud::Point>(0, VK_VERTEX_INPUT_RATE_INSTANCE));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::Read);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::Additive);
}

void RenderPipelineSkyboxStars::setModelMatrix(const Matrix4& value) {
    pushConstants(PushConstant{"modelMatrix", value});
}

void RenderPipelineSkyboxStars::setUniformCamera(const VulkanBuffer& ubo) {
    uniforms[0] = {"Camera", ubo};
}

void RenderPipelineSkyboxStars::setTextureColor(const VulkanTexture& texture) {
    textures[0] = {"colorTexture", texture};
}

void RenderPipelineSkyboxStars::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, textures, {});
}
