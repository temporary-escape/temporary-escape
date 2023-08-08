#include "render_pipeline_shadows_grid.hpp"
#include "../../assets/assets_manager.hpp"
#include "../../scene/camera.hpp"
#include "../mesh_utils.hpp"

using namespace Engine;

RenderPipelineShadowsGrid::RenderPipelineShadowsGrid(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelineShadowsGrid"} {

    addShader(assetsManager.getShaders().find("component_grid_vert"));
    addShader(assetsManager.getShaders().find("component_shadow_frag"));
    addVertexInput(RenderPipeline::VertexInput::of<VoxelShape::VertexFinal>(0));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    setDepthMode(DepthMode::ReadWrite);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::None);
}

void RenderPipelineShadowsGrid::setModelMatrix(const Matrix4& value) {
    pushConstants(PushConstant{"modelMatrix", value});
}

void RenderPipelineShadowsGrid::setNormalMatrix(const Matrix3& value) {
    pushConstants(PushConstant{"normalMatrix", value});
}

void RenderPipelineShadowsGrid::setEntityColor(const Color4& value) {
    pushConstants(PushConstant{"entityColor", value});
}

void RenderPipelineShadowsGrid::setUniformCamera(const VulkanBuffer& ubo, const uint32_t index) {
    uniforms[0] = {"Camera", ubo, index * sizeof(Camera::Uniform), sizeof(Camera::Uniform)};
}

void RenderPipelineShadowsGrid::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, {}, {});
}
