#pragma once

#include "../render_pipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineModelInstanced : public RenderPipeline {
public:
    explicit RenderPipelineModelInstanced(VulkanRenderer& vulkan, AssetsManager& assetsManager);

    void setUniformCamera(const VulkanBuffer& ubo);
    void setUniformMaterial(const VulkanBuffer& ubo);
    void setTextureBaseColor(const VulkanTexture& texture);
    void setTextureEmissive(const VulkanTexture& texture);
    void setTextureMetallicRoughness(const VulkanTexture& texture);
    void setTextureNormal(const VulkanTexture& texture);
    void setTextureAmbientOcclusion(const VulkanTexture& texture);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<UniformBindingRef, 2> uniforms;
    std::array<SamplerBindingRef, 5> textures;
};
} // namespace Engine
