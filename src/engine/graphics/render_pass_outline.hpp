#pragma once

#include "render_pass.hpp"
#include "render_subpass_outline.hpp"

namespace Engine {
class ENGINE_API RenderPassOutline : public RenderPass {
public:
    enum Attachments {
        Color = 0,
    };

    static const size_t totalAttachments = 1;

    explicit RenderPassOutline(VulkanRenderer& vulkan, RenderResources& resources, AssetsManager& assetsManager,
                               const Vector2i& viewport, const VulkanTexture& entityColor);
    virtual ~RenderPassOutline() = default;

    void render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene);

private:
    RenderSubpassOutline subpassOutline;
};
} // namespace Engine
