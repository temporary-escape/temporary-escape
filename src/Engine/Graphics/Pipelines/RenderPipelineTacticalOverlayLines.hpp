#pragma once

#include "../RenderPipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineTacticalOverlayLines : public RenderPipeline {
public:
    explicit RenderPipelineTacticalOverlayLines(VulkanRenderer& vulkan);

    void setModelMatrix(const Matrix4& value);
    void setColor(const Color4& value);
    void setPlayerPos(const Vector3& value);
    void setUniformCamera(const VulkanBuffer& ubo);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<UniformBindingRef, 1> uniforms;
};
} // namespace Engine
