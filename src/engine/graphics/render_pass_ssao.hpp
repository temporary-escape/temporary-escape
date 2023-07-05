#pragma once

#include "render_pass.hpp"
#include "render_subpass_ssao.hpp"

namespace Engine {
class ENGINE_API RenderPassSsao : public RenderPass {
public:
    enum Attachments {
        Ssao = 0,
    };

    static const size_t totalAttachments = 1;

    explicit RenderPassSsao(VulkanRenderer& vulkan, RenderResources& resources, AssetsManager& assetsManager,
                            const Vector2i& viewport, const RenderPassOpaque& previous);
    virtual ~RenderPassSsao() = default;

    void render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene);

private:
    RenderSubpassSsao subpassSsao;
};
} // namespace Engine
