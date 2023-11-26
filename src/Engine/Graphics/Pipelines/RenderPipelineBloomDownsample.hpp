#pragma once

#include "../RenderPipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineBloomDownsample : public RenderPipeline {
public:
    explicit RenderPipelineBloomDownsample(VulkanRenderer& vulkan, AssetsManager& assetsManager);

    void setTexture(const VulkanTexture& texture);
    void setTextureSize(const Vector2& value);
    void setMipLevel(int value);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<SamplerBindingRef, 1> textures;
};
} // namespace Engine
