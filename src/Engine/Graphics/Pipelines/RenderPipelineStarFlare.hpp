#pragma once

#include "../RenderPipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineStarFlare : public RenderPipeline {
public:
    explicit RenderPipelineStarFlare(VulkanRenderer& vulkan);

    void setModelMatrix(const Matrix4& value);
    void setSize(const Vector2& value);
    void setTemp(float value);
    void setUniformCamera(const VulkanBuffer& ubo);
    void setTextureColor(const VulkanTexture& texture);
    void setTextureSpectrumLow(const VulkanTexture& texture);
    void setTextureSpectrumHigh(const VulkanTexture& texture);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<UniformBindingRef, 1> uniforms;
    std::array<SamplerBindingRef, 3> textures;
};
} // namespace Engine
