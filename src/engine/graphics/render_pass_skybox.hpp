#pragma once

#include "render_pass.hpp"
#include "render_subpass_skybox.hpp"

namespace Engine {
class ENGINE_API RenderPassSkybox : public RenderPass {
public:
    enum Attachments {
        Depth = 0,
        Forward = 1,
    };

    static const size_t totalAttachments = 2;

    explicit RenderPassSkybox(VulkanRenderer& vulkan, Registry& registry, const Vector2i& viewport,
                              const VulkanTexture& brdf);
    virtual ~RenderPassSkybox() = default;

    void render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene);

private:
    RenderSubpassSkybox subpassSkybox;
};
} // namespace Engine
