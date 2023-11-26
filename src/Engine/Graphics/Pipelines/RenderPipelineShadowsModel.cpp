#include "RenderPipelineShadowsModel.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Camera.hpp"
#include "../../Scene/Components/ComponentModel.hpp"

using namespace Engine;

RenderPipelineShadowsModel::RenderPipelineShadowsModel(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelineShadowsModel"} {

    addShader(assetsManager.getShaders().find("component_model_vert"));
    addShader(assetsManager.getShaders().find("component_shadow_frag"));
    addVertexInput(RenderPipeline::VertexInput::of<ComponentModel::Vertex>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::ReadWrite);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::None);
    setDepthClamp(DepthClamp::Enabled);
}

void RenderPipelineShadowsModel::setModelMatrix(const Matrix4& value) {
    pushConstants(PushConstant{"modelMatrix", value});
}

void RenderPipelineShadowsModel::setNormalMatrix(const Matrix3& value) {
    pushConstants(PushConstant{"normalMatrix", value});
}

void RenderPipelineShadowsModel::setEntityColor(const Color4& value) {
    pushConstants(PushConstant{"entityColor", value});
}

void RenderPipelineShadowsModel::setUniformCamera(const VulkanBuffer& ubo, const uint32_t index) {
    uniforms[0] = {"Camera", ubo, index * sizeof(Camera::Uniform), sizeof(Camera::Uniform)};
}

void RenderPipelineShadowsModel::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, {}, {});
}
