#pragma once

#include "../RenderPipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineSkybox : public RenderPipeline {
public:
    explicit RenderPipelineSkybox(VulkanRenderer& vulkan, AssetsManager& assetsManager);

    void setModelMatrix(const Matrix4& value);
    void setUniformCamera(const VulkanBuffer& ubo);
    void setTextureBaseColor(const VulkanTexture& texture);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<UniformBindingRef, 1> uniforms;
    std::array<SamplerBindingRef, 1> textures;
};
} // namespace Engine
