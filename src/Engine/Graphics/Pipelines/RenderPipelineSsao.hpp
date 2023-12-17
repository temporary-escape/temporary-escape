#pragma once

#include "../RenderPipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineSSAO : public RenderPipeline {
public:
    explicit RenderPipelineSSAO(VulkanRenderer& vulkan);

    void setKernelSize(const int value);
    void setScale(const Vector2& value);
    void setUniformCamera(const VulkanBuffer& ubo);
    void setUniformSamples(const VulkanBuffer& ubo);
    void setTextureNoise(const VulkanTexture& texture);
    void setTextureDepth(const VulkanTexture& texture);
    void setTextureNormal(const VulkanTexture& texture);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<UniformBindingRef, 2> uniforms;
    std::array<SamplerBindingRef, 3> textures;
};
} // namespace Engine
