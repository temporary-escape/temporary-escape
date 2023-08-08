#pragma once

#include "../assets/assets_manager.hpp"
#include "pipelines/render_pipeline_blit.hpp"
#include "render_buffer_pbr.hpp"
#include "renderer.hpp"

namespace Engine {
class ENGINE_API Scene;

class ENGINE_API RendererScenePbr : public Renderer {
public:
    explicit RendererScenePbr(const RenderOptions& options, VulkanRenderer& vulkan, RenderResources& resources,
                              AssetsManager& assetsManager);

    void setMousePos(const Vector2i& mousePos);
    void render(VulkanCommandBuffer& vkb, Scene& scene) override;
    void transitionForBlit(VulkanCommandBuffer& vkb);
    void blit(VulkanCommandBuffer& vkb);
    Vector2i getViewport() const;
    const VulkanTexture& getFinalBuffer() const;

private:
    const RenderOptions options;
    VulkanRenderer& vulkan;
    RenderResources& resources;
    RenderBufferPbr renderBufferPbr;
    RenderPipelineBlit pipelineBlit;
};
} // namespace Engine
