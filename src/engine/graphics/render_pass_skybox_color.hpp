#pragma once

#include "render_pass.hpp"
#include "render_subpass_skybox_color.hpp"

namespace Engine {
class ENGINE_API RenderPassSkyboxColor : public RenderPass {
public:
    enum Attachments {
        Color = 0,
    };

    static const size_t totalAttachments = 1;

    explicit RenderPassSkyboxColor(VulkanRenderer& vulkan, RenderResources& resources, AssetsManager& assetsManager,
                                   const Vector2i& viewport);
    virtual ~RenderPassSkyboxColor() = default;

    void render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene);

private:
    RenderSubpassSkyboxColor subpassSkyboxColor;
};
} // namespace Engine
