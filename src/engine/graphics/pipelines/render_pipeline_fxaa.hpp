#pragma once

#include "../render_pipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineFXAA : public RenderPipeline {
public:
    explicit RenderPipelineFXAA(VulkanRenderer& vulkan, AssetsManager& assetsManager);

    void setTexture(const VulkanTexture& texture);
    void setTextureSize(const Vector2& value);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<SamplerBindingRef, 1> textures;
};
} // namespace Engine
