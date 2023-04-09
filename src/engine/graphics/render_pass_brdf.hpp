#pragma once

#include "render_pass.hpp"
#include "render_subpass_brdf.hpp"

namespace Engine {
class ENGINE_API RenderPassBrdf : public RenderPass {
public:
    enum Attachments {
        Color = 0,
    };

    static const size_t totalAttachments = 1;

    explicit RenderPassBrdf(VulkanRenderer& vulkan, Registry& registry, const Vector2i& viewport);
    virtual ~RenderPassBrdf() = default;

    void render(VulkanCommandBuffer& vkb, const Vector2i& viewport);

private:
    RenderSubpassBrdf subpassBrdf;
};
} // namespace Engine
