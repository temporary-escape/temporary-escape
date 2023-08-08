#pragma once

#include "../render_pipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelinePositionFeedback : public RenderPipeline {
public:
    explicit RenderPipelinePositionFeedback(VulkanRenderer& vulkan, AssetsManager& assetsManager);

    void setViewport(const Vector2& value);
    void setCount(int value);
    void setUniformCamera(const VulkanBuffer& ubo);
    void setBufferInput(const VulkanBuffer& ubo);
    void setBufferOutput(const VulkanBuffer& ubo);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<UniformBindingRef, 3> uniforms;
};
} // namespace Engine
