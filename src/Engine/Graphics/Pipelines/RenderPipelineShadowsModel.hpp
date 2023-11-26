#pragma once

#include "../RenderPipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineShadowsModel : public RenderPipeline {
public:
    explicit RenderPipelineShadowsModel(VulkanRenderer& vulkan, AssetsManager& assetsManager);

    void setModelMatrix(const Matrix4& value);
    void setNormalMatrix(const Matrix3& value);
    void setEntityColor(const Color4& value);
    void setUniformCamera(const VulkanBuffer& ubo, uint32_t index);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<UniformBindingRef, 1> uniforms;
};
} // namespace Engine
