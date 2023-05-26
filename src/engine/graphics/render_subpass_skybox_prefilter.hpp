#pragma once

#include "../assets/planet_type.hpp"
#include "render_pipeline.hpp"
#include "render_subpass.hpp"

namespace Engine {
class ENGINE_API RenderSubpassSkyboxPrefilter : public RenderSubpass {
public:
    explicit RenderSubpassSkyboxPrefilter(VulkanRenderer& vulkan, AssetsManager& assetsManager);
    virtual ~RenderSubpassSkyboxPrefilter() = default;

    void reset();
    void render(VulkanCommandBuffer& vkb, const VulkanTexture& skyboxColor, const Matrix4& projection,
                const Matrix4& view, float roughness);

private:
    VulkanRenderer& vulkan;

    RenderPipeline pipelinePrefilter;
    Mesh skyboxMesh;
};
} // namespace Engine
