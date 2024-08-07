#pragma once

#include "../RenderPipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelinePlanet : public RenderPipeline {
public:
    explicit RenderPipelinePlanet(VulkanRenderer& vulkan);

    void setModelMatrix(const Matrix4& value);
    void setNormalMatrix(const Matrix3& value);
    void setUniformCamera(const VulkanBuffer& ubo);
    void setUniformAtmosphere(const VulkanBuffer& ubo);
    void setTextureBaseColor(const VulkanTexture& texture);
    void setTextureMetallicRoughness(const VulkanTexture& texture);
    void setTextureNormal(const VulkanTexture& texture);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<UniformBindingRef, 2> uniforms;
    std::array<SamplerBindingRef, 3> textures;
};
} // namespace Engine
