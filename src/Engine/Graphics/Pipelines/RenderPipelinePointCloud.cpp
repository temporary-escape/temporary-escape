#include "RenderPipelinePointCloud.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Components/ComponentPointCloud.hpp"

using namespace Engine;

RenderPipelinePointCloud::RenderPipelinePointCloud(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelinePointCloud"} {

    addShader(assetsManager.getShaders().find("component_point_cloud_vert"));
    addShader(assetsManager.getShaders().find("component_point_cloud_frag"));
    addVertexInput(RenderPipeline::VertexInput::of<ComponentPointCloud::Point>(0, VK_VERTEX_INPUT_RATE_INSTANCE));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
    setDepthMode(DepthMode::Read);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::Normal);
}

void RenderPipelinePointCloud::setModelMatrix(const Matrix4& value) {
    pushConstants(PushConstant{"modelMatrix", value});
}

void RenderPipelinePointCloud::setUniformCamera(const VulkanBuffer& ubo) {
    uniforms[0] = {"Camera", ubo};
}

void RenderPipelinePointCloud::setTextureColor(const VulkanTexture& texture) {
    textures[0] = {"colorTexture", texture};
}

void RenderPipelinePointCloud::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, textures, {});
}
