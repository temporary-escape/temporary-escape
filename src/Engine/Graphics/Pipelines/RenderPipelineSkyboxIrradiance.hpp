#pragma once

#include "../RenderPipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineSkyboxIrradiance : public RenderPipeline {
public:
    explicit RenderPipelineSkyboxIrradiance(VulkanRenderer& vulkan);

    void setProjectionViewMatrix(const Matrix4& value);
    void setTextureSkybox(const VulkanTexture& texture);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<SamplerBindingRef, 1> textures;
};
} // namespace Engine
