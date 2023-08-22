#pragma once

#include "../render_pipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineParticles : public RenderPipeline {
public:
    explicit RenderPipelineParticles(VulkanRenderer& vulkan, AssetsManager& assetsManager);

    void setModelMatrix(const Matrix4& value);
    void setTimeDelta(float value);
    void setUniformCamera(const VulkanBuffer& ubo);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<UniformBindingRef, 1> uniforms;
};
} // namespace Engine
