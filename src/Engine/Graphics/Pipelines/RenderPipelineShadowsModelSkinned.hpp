#pragma once

#include "../RenderPipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineShadowsModelSkinned : public RenderPipeline {
public:
    explicit RenderPipelineShadowsModelSkinned(VulkanRenderer& vulkan);

    void setModelMatrix(const Matrix4& value);
    void setNormalMatrix(const Matrix3& value);
    void setEntityColor(const Color4& value);
    void setUniformCamera(const VulkanBuffer& ubo, uint32_t index);
    void setUniformArmature(const VulkanBuffer& ubo, size_t offset);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<UniformBindingRef, 2> uniforms;
};
} // namespace Engine
