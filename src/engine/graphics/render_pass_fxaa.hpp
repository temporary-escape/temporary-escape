#pragma once

#include "render_pass.hpp"
#include "render_subpass_fxaa.hpp"

namespace Engine {
class ENGINE_API RenderPassFxaa : public RenderPass {
public:
    enum Attachments {
        Color = 0,
    };

    static const size_t totalAttachments = 1;

    explicit RenderPassFxaa(VulkanRenderer& vulkan, RenderResources& resources, AssetsManager& assetsManager,
                            const Vector2i& viewport, const VulkanTexture& forward);
    virtual ~RenderPassFxaa() = default;

    void render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene);

private:
    RenderSubpassFxaa subpassFxaa;
};
} // namespace Engine
