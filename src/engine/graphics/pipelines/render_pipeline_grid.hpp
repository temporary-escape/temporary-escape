#pragma once

#include "../render_pipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineGrid : public RenderPipeline {
public:
    explicit RenderPipelineGrid(VulkanRenderer& vulkan, AssetsManager& assetsManager);

    void setModelMatrix(const Matrix4& value);
    void setNormalMatrix(const Matrix3& value);
    void setEntityColor(const Color4& value);
    void setUniformCamera(const VulkanBuffer& ubo);
    void setUniformMaterial(const VulkanBuffer& ubo);
    void setTextureBaseColor(const VulkanTexture& texture);
    void setTextureEmissive(const VulkanTexture& texture);
    void setTextureMetallicRoughness(const VulkanTexture& texture);
    void setTextureNormal(const VulkanTexture& texture);
    void setTexturePalette(const VulkanTexture& texture);
    void setTextureMask(const VulkanTexture& texture);
    void setTextureAmbientOcclusion(const VulkanTexture& texture);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<UniformBindingRef, 2> uniforms;
    std::array<SamplerBindingRef, 7> textures;
};
} // namespace Engine
