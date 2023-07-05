#pragma once

#include "render_pass.hpp"
#include "render_subpass_combine.hpp"

namespace Engine {
class ENGINE_API RenderPassCombine : public RenderPass {
public:
    enum Attachments {
        Color = 0,
    };

    static const size_t totalAttachments = 1;

    explicit RenderPassCombine(const Config& config, VulkanRenderer& vulkan, RenderResources& resources,
                               AssetsManager& assetsManager, const Vector2i& viewport, const VulkanTexture& dst,
                               const VulkanTexture& color, const VulkanTexture& blured);
    virtual ~RenderPassCombine() = default;

    void render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene);

private:
    RenderSubpassCombine subpassCombine;
};
} // namespace Engine
