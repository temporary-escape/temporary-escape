#pragma once

#include "../RenderPipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineModelSkinned : public RenderPipeline {
public:
    explicit RenderPipelineModelSkinned(VulkanRenderer& vulkan);

    void setModelMatrix(const Matrix4& value);
    void setNormalMatrix(const Matrix3& value);
    void setEntityColor(const Color4& value);
    void setUniformCamera(const VulkanBuffer& ubo);
    void setUniformArmature(const VulkanBuffer& ubo, size_t offset);
    void setUniformMaterial(const VulkanBuffer& ubo);
    void setTextureBaseColor(const VulkanTexture& texture);
    void setTextureEmissive(const VulkanTexture& texture);
    void setTextureMetallicRoughness(const VulkanTexture& texture);
    void setTextureNormal(const VulkanTexture& texture);
    void setTextureAmbientOcclusion(const VulkanTexture& texture);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<UniformBindingRef, 3> uniforms;
    std::array<SamplerBindingRef, 5> textures;
};
} // namespace Engine
