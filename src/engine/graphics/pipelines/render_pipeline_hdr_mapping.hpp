#pragma once

#include "../render_pipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineHDRMapping : public RenderPipeline {
public:
    explicit RenderPipelineHDRMapping(VulkanRenderer& vulkan, AssetsManager& assetsManager);

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
