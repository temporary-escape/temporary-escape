#pragma once

#include "../render_pipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineBulletsTrail : public RenderPipeline {
public:
    explicit RenderPipelineBulletsTrail(VulkanRenderer& vulkan, AssetsManager& assetsManager);

    void setUniformCamera(const VulkanBuffer& ubo);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<UniformBindingRef, 1> uniforms;
};
} // namespace Engine
