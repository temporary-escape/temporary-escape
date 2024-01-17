#pragma once

#include "../RenderPipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineSpaceDust : public RenderPipeline {
public:
    explicit RenderPipelineSpaceDust(VulkanRenderer& vulkan);

    void setModelMatrix(const Matrix4& value);
    void setMoveDirection(const Vector3& value);
    void setUniformCamera(const VulkanBuffer& ubo);
    void setTextureColor(const VulkanTexture& texture);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<UniformBindingRef, 1> uniforms;
    std::array<SamplerBindingRef, 1> textures;
};
} // namespace Engine
