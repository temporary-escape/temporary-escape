#include "RenderPipelineParticles.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Components/ComponentParticles.hpp"

using namespace Engine;

RenderPipelineParticles::RenderPipelineParticles(VulkanRenderer& vulkan, AssetsManager& assetsManager) :
    RenderPipeline{vulkan, "RenderPipelineParticles"} {

    addShader(assetsManager.getShaders().find("component_particles_vert"));
    addShader(assetsManager.getShaders().find("component_particles_frag"));
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
    setDepthMode(DepthMode::Read);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::Additive);
}

void RenderPipelineParticles::setModelMatrix(const Matrix4& value) {
    pushConstants(PushConstant{"modelMatrix", value});
}

void RenderPipelineParticles::setTimeDelta(const float value) {
    pushConstants(PushConstant{"timeDelta", value});
}

void RenderPipelineParticles::setOverrideStrength(const float value) {
    pushConstants(PushConstant{"overrideStrength", value});
}

void RenderPipelineParticles::setOverrideAlpha(const float value) {
    pushConstants(PushConstant{"overrideAlpha", value});
}

void RenderPipelineParticles::setUniformCamera(const VulkanBuffer& ubo) {
    uniforms[0] = {"Camera", ubo};
}

void RenderPipelineParticles::setUniformParticlesType(const Engine::VulkanBuffer& ubo) {
    uniforms[1] = {"ParticlesType", ubo};
}

void RenderPipelineParticles::setTextureColor(const VulkanTexture& texture) {
    textures[0] = {"colorTexture", texture};
}

void RenderPipelineParticles::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, textures, {});
}
