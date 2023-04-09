#pragma once

#include "render_pass.hpp"
#include "render_subpass_pbr.hpp"

namespace Engine {
class ENGINE_API RenderPassLighting : public RenderPass {
public:
    enum Attachments {
        Forward = 0,
    };

    static const size_t totalAttachments = 1;

    explicit RenderPassLighting(VulkanRenderer& vulkan, Registry& registry, const Vector2i& viewport,
                                const RenderPassOpaque& opaque, const RenderPassSsao& ssao, const VulkanTexture& brdf,
                                const VulkanTexture& forward);
    virtual ~RenderPassLighting() = default;

    void render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene);

private:
    RenderSubpassPbr subpassPbr;
};
} // namespace Engine
