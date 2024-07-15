#include "RenderPipelineParticles.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Components/ComponentParticles.hpp"
#include <component_particles_frag.spirv.h>
#include <component_particles_vert.spirv.h>

using namespace Engine;

RenderPipelineParticles::RenderPipelineParticles(VulkanRenderer& vulkan) :
    RenderPipeline{vulkan, "RenderPipelineParticles"} {

    addShader(Embed::component_particles_vert_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
    addShader(Embed::component_particles_frag_spirv, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
    setTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
    setDepthMode(DepthMode::Read);
    setPolygonMode(VkPolygonMode::VK_POLYGON_MODE_FILL);
    setCullMode(VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
    setFrontFace(VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setBlending(Blending::Additive);
}

void RenderPipelineParticles::setUniformCamera(const VulkanBuffer& ubo) {
    uniforms[0] = {"Camera", ubo};
}

void RenderPipelineParticles::setUniformParticlesTypes(const Engine::VulkanBuffer& ubo) {
    uniforms[1] = {"ParticlesTypes", ubo};
}

void RenderPipelineParticles::setUniformBatch(const VulkanBuffer& ubo, const size_t index) {
    uniforms[2] = {
        "ParticlesBatch",
        ubo,
        sizeof(ComponentParticles::ParticlesBatchUniform) * index,
        sizeof(ComponentParticles::ParticlesBatchUniform),
    };
}

void RenderPipelineParticles::setTextureColor(const VulkanTexture& texture) {
    textures[0] = {"colorTexture", texture};
}

void RenderPipelineParticles::flushDescriptors(VulkanCommandBuffer& vkb) {
    bindDescriptors(vkb, uniforms, textures, {});
}
