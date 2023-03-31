#pragma once

#include "render_pass.hpp"
#include "render_subpass_forward.hpp"

namespace Engine {
class ENGINE_API RenderPassOpaque;
class ENGINE_API RenderPassLighting;

class ENGINE_API RenderPassForward : public RenderPass {
public:
    enum Attachments {
        Depth = 0,
        Forward = 1,
    };

    static const size_t totalAttachments = 2;

    explicit RenderPassForward(VulkanRenderer& vulkan, Registry& registry, const Vector2i& viewport,
                               const RenderPassOpaque& opaque, const RenderPassLighting& ssao);
    virtual ~RenderPassForward() = default;

    void render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene);

private:
    RenderSubpassForward subpassForward;
};
} // namespace Engine
