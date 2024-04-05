#pragma once

#include "../RenderPipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineSkyboxPlanet : public RenderPipeline {
public:
    explicit RenderPipelineSkyboxPlanet(VulkanRenderer& vulkan);

    void setModelMatrix(const Matrix4& value);
    void setNormalMatrix(const Matrix3& value);
    void setUniformCamera(const VulkanBuffer& ubo);
    void setUniformAtmosphere(const VulkanBuffer& ubo);
    void setUniformDirectionalLights(const VulkanBuffer& ubo);
    void setTextureAlbedo(const VulkanTexture& texture);
    void setTextureNormal(const VulkanTexture& texture);
    void setTextureMetallicRoughness(const VulkanTexture& texture);
    void setTextureSkyboxIrradiance(const VulkanTexture& texture);
    void setTextureSkyboxPrefilter(const VulkanTexture& texture);
    void setTextureBrdf(const VulkanTexture& texture);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<UniformBindingRef, 3> uniforms{};
    std::array<SamplerBindingRef, 6> textures{};
};
} // namespace Engine
