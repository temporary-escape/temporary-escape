#pragma once

#include "../render_pipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineBlit : public RenderPipeline {
public:
    explicit RenderPipelineBlit(VulkanRenderer& vulkan, AssetsManager& assetsManager);

    void setTexture(const VulkanTexture& texture);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<SamplerBindingRef, 1> textures;
};
} // namespace Engine
