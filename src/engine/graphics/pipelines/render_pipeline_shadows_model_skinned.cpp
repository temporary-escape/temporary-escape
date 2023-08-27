#include "render_pipeline_shadows_model_skinned.hpp"
#include "../../assets/assets_manager.hpp"
#include "../../scene/camera.hpp"
#include "../../scene/components/component_model_skinned.hpp"

using namespace Engine;

RenderPipelineShadowsModelSkinned::RenderPipelineShadowsModelSkinned(VulkanRenderer& vulkan,
                                                                     AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelineShadowsModelSkinned"} {

    addShader(assetsManager.getShaders().find("component_model_skinned_vert"));
    addShader(assetsManager.getShaders().find("component_shadow_frag"));
    addVertexInput(RenderPipeline::VertexInput::of<ComponentModelSkinned::Vertex>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::ReadWrite);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::None);
    setDepthClamp(DepthClamp::Enabled);
}

void RenderPipelineShadowsModelSkinned::setModelMatrix(const Matrix4& value) {
    pushConstants(PushConstant{"modelMatrix", value});
}

void RenderPipelineShadowsModelSkinned::setNormalMatrix(const Matrix3& value) {
    pushConstants(PushConstant{"normalMatrix", value});
}

void RenderPipelineShadowsModelSkinned::setEntityColor(const Color4& value) {
    pushConstants(PushConstant{"entityColor", value});
}

void RenderPipelineShadowsModelSkinned::setUniformCamera(const VulkanBuffer& ubo, const uint32_t index) {
    uniforms[0] = {"Camera", ubo, index * sizeof(Camera::Uniform), sizeof(Camera::Uniform)};
}

void RenderPipelineShadowsModelSkinned::setUniformArmature(const VulkanBuffer& ubo, const size_t offset) {
    uniforms[1] = {"Armature", ubo, offset, sizeof(ComponentModelSkinned::Armature)};
}

void RenderPipelineShadowsModelSkinned::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, {}, {});
}
