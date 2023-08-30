#pragma once

#include "../render_pipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineParticles : public RenderPipeline {
public:
    explicit RenderPipelineParticles(VulkanRenderer& vulkan, AssetsManager& assetsManager);

    void setModelMatrix(const Matrix4& value);
    void setTimeDelta(float value);
    void setOverrideStrength(float value);
    void setOverrideAlpha(float value);
    void setUniformCamera(const VulkanBuffer& ubo);
    void setUniformParticlesType(const VulkanBuffer& ubo);
    void setTextureColor(const VulkanTexture& texture);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<UniformBindingRef, 2> uniforms;
    std::array<SamplerBindingRef, 1> textures;
};
} // namespace Engine
