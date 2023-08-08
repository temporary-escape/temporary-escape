#pragma once

#include "../render_pipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineSkyboxNebula : public RenderPipeline {
public:
    explicit RenderPipelineSkyboxNebula(VulkanRenderer& vulkan, AssetsManager& assetsManager);

    void setUniformCamera(const VulkanBuffer& ubo);
    void setUniformNebula(const VulkanBuffer& ubo);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<UniformBindingRef, 2> uniforms;
};
} // namespace Engine
