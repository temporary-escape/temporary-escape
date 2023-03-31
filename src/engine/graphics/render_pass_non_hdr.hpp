#pragma once

#include "render_pass.hpp"
#include "render_subpass_non_hdr.hpp"

namespace Engine {
class ENGINE_API RenderPassForward;

class ENGINE_API RenderPassNonHdr : public RenderPass {
public:
    enum Attachments {
        Depth = 0,
        Forward = 1,
    };

    static const size_t totalAttachments = 2;

    explicit RenderPassNonHdr(VulkanRenderer& vulkan, Registry& registry, const Vector2i& viewport,
                              const RenderPassForward& forward);
    virtual ~RenderPassNonHdr() = default;

    void render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene);

private:
    RenderSubpassNonHdr subpassNonHdr;
};
} // namespace Engine
