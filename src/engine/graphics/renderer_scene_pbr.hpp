#pragma once

#include "../assets/assets_manager.hpp"
#include "render_buffer_pbr.hpp"
#include "renderer.hpp"

namespace Engine {
class ENGINE_API Scene;

class ENGINE_API RendererScenePbr : public Renderer {
public:
    explicit RendererScenePbr(const RenderOptions& options, VulkanRenderer& vulkan, RenderResources& resources,
                              AssetsManager& assetsManager);

    void render(VulkanCommandBuffer& vkb, Scene& scene);
    void blit(VulkanCommandBuffer& vkb);

private:
    RenderBufferPbr renderBufferPbr;
};
} // namespace Engine
