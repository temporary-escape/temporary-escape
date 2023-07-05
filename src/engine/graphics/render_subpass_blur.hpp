#pragma once

#include "render_pipeline.hpp"
#include "render_subpass.hpp"

namespace Engine {
class ENGINE_API RenderSubpassBlur : public RenderSubpass {
public:
    explicit RenderSubpassBlur(VulkanRenderer& vulkan, RenderResources& resources, AssetsManager& assetsManager,
                               const VulkanTexture& color, bool vertical);
    virtual ~RenderSubpassBlur() = default;

    void reset();
    void render(VulkanCommandBuffer& vkb);

private:
    struct GaussianWeightsUniform {
        Vector4 weight[32];
        int count{0};
    };

    void createGaussianKernel(const size_t size, double sigma);

    VulkanRenderer& vulkan;
    RenderResources& resources;
    const VulkanTexture& color;
    const bool vertical;
    RenderPipeline pipelineBlur;
    VulkanBuffer weightsUbo;
};
} // namespace Engine
