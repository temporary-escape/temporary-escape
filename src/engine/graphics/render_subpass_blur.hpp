#pragma once

#include "render_pipeline.hpp"
#include "render_subpass.hpp"

namespace Engine {
class ENGINE_API RenderSubpassBlur : public RenderSubpass {
public:
    explicit RenderSubpassBlur(VulkanRenderer& vulkan, Registry& registry, const VulkanTexture& color, bool vertical);
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
    const VulkanTexture& color;
    const bool vertical;
    RenderPipeline pipelineBlur;
    Mesh fullScreenQuad;
    VulkanBuffer weightsUbo;
};
} // namespace Engine
