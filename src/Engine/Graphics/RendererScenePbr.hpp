#pragma once

#include "../Assets/AssetsManager.hpp"
#include "Pipelines/RenderPipelineBlit.hpp"
#include "RenderBufferPbr.hpp"
#include "Renderer.hpp"

namespace Engine {
class ENGINE_API Scene;

class ENGINE_API RendererScenePbr : public Renderer {
public:
    explicit RendererScenePbr(const RenderOptions& options, VulkanRenderer& vulkan, RenderResources& resources);

    void setMousePos(const Vector2i& mousePos);
    void render(VulkanCommandBuffer& vkb, VulkanCommandBuffer& vkbc, Scene& scene) override;
    void transitionForBlit(VulkanCommandBuffer& vkb);
    void blit(VulkanCommandBuffer& vkb);
    Vector2i getViewport() const;
    const VulkanTexture& getFinalBuffer() const;
    bool isBlitReady() const {
        return blitReady;
    }

private:
    const RenderOptions options;
    VulkanRenderer& vulkan;
    RenderResources& resources;
    RenderBufferPbr renderBufferPbr;
    RenderPipelineBlit pipelineBlit;
    bool blitReady{false};
};
} // namespace Engine
