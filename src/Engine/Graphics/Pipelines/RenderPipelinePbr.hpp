#pragma once

#include "../RenderPipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelinePbr : public RenderPipeline {
public:
    explicit RenderPipelinePbr(VulkanRenderer& vulkan);

    void setUniformCamera(const VulkanBuffer& ubo);
    void setUniformDirectionalLights(const VulkanBuffer& ubo);
    void setUniformShadowsViewProj(const VulkanBuffer& ubo);
    void setTextureIrradiance(const VulkanTexture& texture);
    void setTexturePrefilter(const VulkanTexture& texture);
    void setTextureBrdf(const VulkanTexture& texture);
    void setTextureDepth(const VulkanTexture& texture);
    void setTextureBaseColorAmbient(const VulkanTexture& texture);
    void setTextureEmissiveRoughness(const VulkanTexture& texture);
    void setTextureNormalMetallic(const VulkanTexture& texture);
    void setTextureSSAO(const VulkanTexture& texture);
    void setTextureShadows(const VulkanTexture& texture);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<UniformBindingRef, 3> uniforms;
    std::array<SamplerBindingRef, 9> textures;
};
} // namespace Engine
