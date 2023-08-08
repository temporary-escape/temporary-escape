#include "render_pipeline_shadows_model_instanced.hpp"
#include "../../assets/assets_manager.hpp"
#include "../../scene/camera.hpp"
#include "../../scene/components/component_model.hpp"
#include "../mesh_utils.hpp"

using namespace Engine;

RenderPipelineShadowsModelInstanced::RenderPipelineShadowsModelInstanced(VulkanRenderer& vulkan,
                                                                         AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelineShadowsModelInstanced"} {

    addShader(assetsManager.getShaders().find("component_model_instanced_vert"));
    addShader(assetsManager.getShaders().find("component_shadow_frag"));
    addVertexInput(RenderPipeline::VertexInput::of<ComponentModel::Vertex>(0));
    addVertexInput(RenderPipeline::VertexInput::of<ComponentModel::InstancedVertex>(
        1, VkVertexInputRate::VK_VERTEX_INPUT_RATE_INSTANCE));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::ReadWrite);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::None);
    setDepthClamp(DepthClamp::Enabled);
}

void RenderPipelineShadowsModelInstanced::setUniformCamera(const VulkanBuffer& ubo, const uint32_t index) {
    uniforms[0] = {"Camera", ubo, index * sizeof(Camera::Uniform), sizeof(Camera::Uniform)};
}

void RenderPipelineShadowsModelInstanced::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, {}, {});
}
