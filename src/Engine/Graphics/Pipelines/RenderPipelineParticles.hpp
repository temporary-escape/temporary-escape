#pragma once

#include "../RenderPipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineParticles : public RenderPipeline {
public:
    explicit RenderPipelineParticles(VulkanRenderer& vulkan);

    void setUniformCamera(const VulkanBuffer& ubo);
    void setUniformParticlesTypes(const VulkanBuffer& ubo);
    void setUniformBatch(const VulkanBuffer& ubo, size_t index);
    void setTextureColor(const VulkanTexture& texture);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<UniformBindingRef, 3> uniforms;
    std::array<SamplerBindingRef, 1> textures;
};
} // namespace Engine
