#pragma once

#include "../RenderPipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineSkyboxNebula : public RenderPipeline {
public:
    explicit RenderPipelineSkyboxNebula(VulkanRenderer& vulkan);

    void setUniformCamera(const VulkanBuffer& ubo);
    void setUniformNebula(const VulkanBuffer& ubo);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<UniformBindingRef, 2> uniforms;
};
} // namespace Engine
