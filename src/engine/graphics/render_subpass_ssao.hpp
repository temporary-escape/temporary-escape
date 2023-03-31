#pragma once

#include "render_pipeline.hpp"
#include "render_subpass.hpp"

namespace Engine {
class ENGINE_API RenderPassOpaque;

class ENGINE_API RenderSubpassSsao : public RenderSubpass {
public:
    explicit RenderSubpassSsao(VulkanRenderer& vulkan, Registry& registry, const RenderPassOpaque& previous);
    virtual ~RenderSubpassSsao() = default;

    void render(VulkanCommandBuffer& vkb, Scene& scene);

private:
    void createSsaoNoise();
    void createSsaoSamples();

    VulkanRenderer& vulkan;
    const RenderPassOpaque& previous;
    RenderPipeline pipelineSsao;

    struct {
        VulkanBuffer ubo;
        VulkanTexture noise;
    } ssaoSamples;

    Mesh fullScreenQuad;
};
} // namespace Engine
