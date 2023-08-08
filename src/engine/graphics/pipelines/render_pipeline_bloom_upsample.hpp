#pragma once

#include "../render_pipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineBloomUpsample : public RenderPipeline {
public:
    explicit RenderPipelineBloomUpsample(VulkanRenderer& vulkan, AssetsManager& assetsManager);

    void setTexture(const VulkanTexture& texture);
    void setFilterRadius(float value);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<SamplerBindingRef, 1> textures;
};
} // namespace Engine
