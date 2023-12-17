#pragma once

#include "../RenderPipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineLines : public RenderPipeline {
public:
    explicit RenderPipelineLines(VulkanRenderer& vulkan);

    void setModelMatrix(const Matrix4& value);
    void setColor(const Color4& value);
    void setUniformCamera(const VulkanBuffer& ubo);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<UniformBindingRef, 1> uniforms;
};
} // namespace Engine
