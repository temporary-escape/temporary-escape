#pragma once

#include "../render_pipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineSkyboxPrefilter : public RenderPipeline {
public:
    explicit RenderPipelineSkyboxPrefilter(VulkanRenderer& vulkan, AssetsManager& assetsManager);

    void setProjectionViewMatrix(const Matrix4& value);
    void setTextureSkybox(const VulkanTexture& texture);
    void setRoughness(float value);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<SamplerBindingRef, 1> textures;
};
} // namespace Engine
