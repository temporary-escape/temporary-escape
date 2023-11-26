#pragma once

#include "../RenderPipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineWorldText : public RenderPipeline {
public:
    explicit RenderPipelineWorldText(VulkanRenderer& vulkan, AssetsManager& assetsManager);

    void setModelMatrix(const Matrix4& value);
    void setColor(const Color4& value);
    void setUniformCamera(const VulkanBuffer& ubo);
    void setTextureColor(const VulkanTexture& texture);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<UniformBindingRef, 1> uniforms;
    std::array<SamplerBindingRef, 1> textures;
};
} // namespace Engine
