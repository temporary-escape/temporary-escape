#pragma once

#include "render_pipeline.hpp"
#include "render_subpass.hpp"

namespace Engine {
class ENGINE_API RenderPassOpaque;
class ENGINE_API RenderPassSsao;
class ENGINE_API RenderPassShadows;

class ENGINE_API RenderSubpassPbr : public RenderSubpass {
public:
    struct DirectionalLights {
        Vector4 colors[4];
        Vector4 directions[4];
        int count{0};
    };

    explicit RenderSubpassPbr(VulkanRenderer& vulkan, RenderResources& resources, AssetsManager& assetsManager,
                              const RenderPassOpaque& opaque, const RenderPassSsao& ssao,
                              const RenderPassShadows& shadows, const VulkanTexture& brdf);
    virtual ~RenderSubpassPbr() = default;

    void render(VulkanCommandBuffer& vkb, Scene& scene);

private:
    void updateDirectionalLights(Scene& scene);

    VulkanRenderer& vulkan;
    RenderResources& resources;
    const RenderPassOpaque& opaque;
    const RenderPassSsao& ssao;
    const RenderPassShadows& shadows;
    const VulkanTexture& brdf;
    SkyboxTextures defaultSkybox;
    RenderPipeline pipelinePbr;
    VulkanDoubleBuffer directionalLightsUbo;
};
} // namespace Engine
