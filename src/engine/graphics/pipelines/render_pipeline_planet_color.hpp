#pragma once

#include "../render_pipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelinePlanetColor : public RenderPipeline {
public:
    explicit RenderPipelinePlanetColor(VulkanRenderer& vulkan, AssetsManager& assetsManager);

    void setTextureBiome(const VulkanTexture& texture);
    void setTextureRoughness(const VulkanTexture& texture);
    void setInputHeightmap(const VulkanTexture& texture);
    void setInputMoisture(const VulkanTexture& texture);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<SamplerBindingRef, 2> textures{};
    std::array<SubpassInputBindingRef, 2> inputs{};
};
} // namespace Engine
