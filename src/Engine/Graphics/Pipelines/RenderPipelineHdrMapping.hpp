#pragma once

#include "../RenderPipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineHDRMapping : public RenderPipeline {
public:
    explicit RenderPipelineHDRMapping(VulkanRenderer& vulkan);

    void setBloomStrength(float value);
    void setGamma(float value);
    void setExposure(float value);
    void setTextureColor(const VulkanTexture& texture);
    void setTextureBloom(const VulkanTexture& texture);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<SamplerBindingRef, 2> textures;
};
} // namespace Engine
