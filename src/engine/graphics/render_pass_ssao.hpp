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

    explicit RenderPassSsao(const Config& config, VulkanRenderer& vulkan, RenderResources& resources,
                            AssetsManager& assetsManager, const Vector2i& viewport, const RenderPassOpaque& previous);
    virtual ~RenderPassSsao() = default;

    void render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene);

private:
    const Config& config;
    RenderSubpassSsao subpassSsao;
};
} // namespace Engine
