#pragma once

#include "../render_pipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelinePointCloud : public RenderPipeline {
public:
    explicit RenderPipelinePointCloud(VulkanRenderer& vulkan, AssetsManager& assetsManager);

    void setModelMatrix(const Matrix4& value);
    void setUniformCamera(const VulkanBuffer& ubo);
    void setTextureColor(const VulkanTexture& texture);
    void flushDescriptors(VulkanCommandBuffer& vkb);

private:
    std::array<UniformBindingRef, 1> uniforms;
    std::array<SamplerBindingRef, 1> textures;
};
} // namespace Engine