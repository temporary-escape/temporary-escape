#pragma once

#include "../render_pipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineShadowsModelInstanced : public RenderPipeline {
public:
    explicit RenderPipelineShadowsModelInstanced(VulkanRenderer& vulkan, AssetsManager& assetsManager);

    void setUniformCamera(const VulkanBuffer& ubo, uint32_t index);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<UniformBindingRef, 1> uniforms;
};
} // namespace Engine
