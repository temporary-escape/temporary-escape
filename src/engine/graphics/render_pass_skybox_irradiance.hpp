#pragma once

#include "render_pass.hpp"
#include "render_subpass_skybox_irradiance.hpp"

namespace Engine {
class ENGINE_API RenderPassSkyboxIrradiance : public RenderPass {
public:
    enum Attachments {
        Irradiance = 0,
    };

    static const size_t totalAttachments = 1;

    explicit RenderPassSkyboxIrradiance(VulkanRenderer& vulkan, RenderResources& resources,
                                        AssetsManager& assetsManager, const Vector2i& viewport);
    virtual ~RenderPassSkyboxIrradiance() = default;

    void render(VulkanCommandBuffer& vkb, const Vector2i& viewport, const VulkanTexture& skyboxColor,
                const Matrix4& projection, const Matrix4& view);

private:
    RenderSubpassSkyboxIrradiance subpassSkyboxIrradiance;
};
} // namespace Engine
