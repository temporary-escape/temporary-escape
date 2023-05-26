#pragma once

#include "../config.hpp"
#include "../font/font_family.hpp"
#include "../scene/scene.hpp"
#include "../vulkan/vulkan_renderer.hpp"
#include "canvas.hpp"
#include "nuklear.hpp"
#include "render_pass_bloom.hpp"
#include "render_pass_brdf.hpp"
#include "render_pass_combine.hpp"
#include "render_pass_compute.hpp"
#include "render_pass_forward.hpp"
#include "render_pass_fxaa.hpp"
#include "render_pass_lighting.hpp"
#include "render_pass_non_hdr.hpp"
#include "render_pass_opaque.hpp"
#include "render_pass_skybox.hpp"
#include "render_pass_ssao.hpp"
#include "skybox.hpp"

namespace Engine {
class ENGINE_API VoxelShapeCache;

class ENGINE_API Renderer {
public:
    explicit Renderer(const Config& config, const Vector2i& viewport, VulkanRenderer& vulkan, Canvas& canvas,
                      Nuklear& nuklear, VoxelShapeCache& voxelShapeCache, AssetsManager& assetsManager,
                      FontFamily& font);
    virtual ~Renderer();

    void render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene);
    void render(const std::shared_ptr<Block>& block, VoxelShape::Type shape);

    void blit(VulkanCommandBuffer& vkb, const Vector2i& viewport);

    VulkanRenderer& getVulkan() {
        return vulkan;
    }

    const Vector2i& getViewport() const {
        return lastViewportSize;
    }

    const VulkanTexture& getTexture() const;

private:
    void renderOneTime(Scene& scene);

    void createRenderPasses(const Vector2i& viewport);
    void createPipelineBlit();
    void renderBrdf();

    const Config& config;
    VulkanRenderer& vulkan;
    Canvas& canvas;
    Nuklear& nuklear;
    VoxelShapeCache& voxelShapeCache;
    AssetsManager& assetsManager;
    FontFamily& font;
    Vector2i lastViewportSize;

    struct {
        std::unique_ptr<RenderPassBrdf> brdf;
        std::unique_ptr<RenderPassCompute> compute;
        std::unique_ptr<RenderPassSkybox> skybox;
        std::unique_ptr<RenderPassOpaque> opaque;
        std::unique_ptr<RenderPassSsao> ssao;
        std::unique_ptr<RenderPassLighting> lighting;
        std::unique_ptr<RenderPassForward> forward;
        std::unique_ptr<RenderPassFxaa> fxaa;
        std::unique_ptr<RenderPassBloom> bloom;
        std::unique_ptr<RenderPassCombine> combine;
        std::unique_ptr<RenderPassNonHdr> nonHdr;
    } renderPasses;

    std::unique_ptr<RenderPipeline> pipelineBlit;
    Mesh fullScreenQuad;
};
} // namespace Engine
