#pragma once

#include "../assets/planet_type.hpp"
#include "render_pipeline.hpp"
#include "render_subpass.hpp"

namespace Engine {
class ENGINE_API RenderSubpassSkyboxColor : public RenderSubpass {
public:
    explicit RenderSubpassSkyboxColor(VulkanRenderer& vulkan, AssetsManager& assetsManager);
    virtual ~RenderSubpassSkyboxColor() = default;

    void render(VulkanCommandBuffer& vkb, Scene& scene);

private:
    void renderNebulas(VulkanCommandBuffer& vkb, Scene& scene);
    void renderPointClouds(VulkanCommandBuffer& vkb, Scene& scene);

    VulkanRenderer& vulkan;

    RenderPipeline pipelineNebula;
    RenderPipeline pipelinePointCloud;
    Mesh skyboxMesh;
};
} // namespace Engine
