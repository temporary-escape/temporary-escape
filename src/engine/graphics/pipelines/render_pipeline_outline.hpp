#pragma once

#include "../render_pipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineOutline : public RenderPipeline {
public:
    explicit RenderPipelineOutline(VulkanRenderer& vulkan, AssetsManager& assetsManager);

    void setColorSelected(const Color4& value);
    void setColorFinal(const Color4& value);
    void setThickness(float value);
    void setTextureEntity(const VulkanTexture& texture);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<SamplerBindingRef, 1> textures{};
};
} // namespace Engine
