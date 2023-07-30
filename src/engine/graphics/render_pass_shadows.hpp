#pragma once

#include "render_pass.hpp"
#include "render_subpass_shadows.hpp"

namespace Engine {
class ENGINE_API RenderPassShadows {
public:
    class ENGINE_API Internal : public RenderPass {
    public:
        enum Attachments { Shadows };

        static const size_t totalAttachments = 1;

        explicit Internal(VulkanRenderer& vulkan, RenderResources& resources, AssetsManager& assetsManager,
                          const Vector2i& viewport, VulkanTexture& depthTexture, VulkanImageView& depthImageView,
                          size_t index);
        virtual ~Internal() = default;

        void render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene);

    private:
        RenderSubpassShadows subpassShadows;
    };

    explicit RenderPassShadows(VulkanRenderer& vulkan, RenderResources& resources, AssetsManager& assetsManager,
                               const Vector2i& viewport);
    virtual ~RenderPassShadows() = default;

    void render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene);

    const VulkanTexture& getShadowMapArray() const {
        return shadowMapArray;
    }

private:
    VulkanTexture shadowMapArray;
    std::array<VulkanImageView, 4> shadowMapViews;
    std::array<std::unique_ptr<Internal>, 4> internal;
};
} // namespace Engine
