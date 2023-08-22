#include "render_pipeline_particles.hpp"
#include "../../assets/assets_manager.hpp"
#include "../../scene/components/component_particles.hpp"

using namespace Engine;

RenderPipelineParticles::RenderPipelineParticles(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelineParticles"} {

    addShader(assetsManager.getShaders().find("component_particles_vert"));
    addShader(assetsManager.getShaders().find("component_particles_frag"));
    addVertexInput(RenderPipeline::VertexInput::of<ComponentParticles::Vertex>(0, VK_VERTEX_INPUT_RATE_INSTANCE));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
    setDepthMode(DepthMode::Read);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::Normal);
}

void RenderPipelineParticles::setModelMatrix(const Matrix4& value) {
    pushConstants(PushConstant{"modelMatrix", value});
}

void RenderPipelineParticles::setTimeDelta(float value) {
    pushConstants(PushConstant{"timeDelta", value});
}

void RenderPipelineParticles::setUniformCamera(const VulkanBuffer& ubo) {
    uniforms[0] = {"Camera", ubo};
}

void RenderPipelineParticles::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, {}, {});
}
