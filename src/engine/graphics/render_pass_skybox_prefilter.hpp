#pragma once

#include "render_pass.hpp"
#include "render_subpass_skybox_prefilter.hpp"

namespace Engine {
class ENGINE_API RenderPassSkyboxPrefilter : public RenderPass {
public:
    enum Attachments {
        Prefilter = 0,
    };

    static const size_t totalAttachments = 1;

    explicit RenderPassSkyboxPrefilter(VulkanRenderer& vulkan, RenderResources& resources, AssetsManager& assetsManager,
                                       const Vector2i& viewport);
    virtual ~RenderPassSkyboxPrefilter() = default;

    void render(VulkanCommandBuffer& vkb, const Vector2i& viewport, const VulkanTexture& skyboxColor,
                const Matrix4& projection, const Matrix4& view);

private:
    RenderSubpassSkyboxPrefilter subpassSkyboxPrefilter;
};
} // namespace Engine
