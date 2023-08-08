#pragma once

#include "../render_pipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelinePlanetNormal : public RenderPipeline {
public:
    explicit RenderPipelinePlanetNormal(VulkanRenderer& vulkan, AssetsManager& assetsManager);

    void setTextureHeight(const VulkanTexture& texture);
    void setResolution(float value);
    void setWaterLevel(float value);
    void setStrength(float value);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<SamplerBindingRef, 1> textures{};
};
} // namespace Engine
