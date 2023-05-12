#pragma once

#include "../assets/planet_type.hpp"
#include "render_pipeline.hpp"
#include "render_subpass.hpp"

namespace Engine {
class ENGINE_API RenderSubpassSkyboxIrradiance : public RenderSubpass {
public:
    explicit RenderSubpassSkyboxIrradiance(VulkanRenderer& vulkan, Registry& registry);
    virtual ~RenderSubpassSkyboxIrradiance() = default;

    void reset();
    void render(VulkanCommandBuffer& vkb, const VulkanTexture& skyboxColor, const Matrix4& projection,
                const Matrix4& view);

private:
    VulkanRenderer& vulkan;

    RenderPipeline pipelineIrradiance;
    Mesh skyboxMesh;
};
} // namespace Engine
