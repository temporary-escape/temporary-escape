#pragma once

#include "../RenderPipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineSkyboxStars : public RenderPipeline {
public:
    explicit RenderPipelineSkyboxStars(VulkanRenderer& vulkan);

    void setModelMatrix(const Matrix4& value);
    void setUniformCamera(const VulkanBuffer& ubo);
    void setTextureColor(const VulkanTexture& texture);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<UniformBindingRef, 1> uniforms;
    std::array<SamplerBindingRef, 1> textures;
};
} // namespace Engine
