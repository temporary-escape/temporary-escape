#pragma once

#include "../../assets/texture.hpp"
#include "../pipelines/render_pipeline_grid.hpp"
#include "../pipelines/render_pipeline_model.hpp"
#include "../pipelines/render_pipeline_model_instanced.hpp"
#include "../render_buffer_pbr.hpp"
#include "../render_pass.hpp"

namespace Engine {
class ENGINE_API RenderPassOpaque : public RenderPass {
public:
    explicit RenderPassOpaque(VulkanRenderer& vulkan, RenderBufferPbr& buffer, RenderResources& resources,
                              AssetsManager& assetsManager);

    void beforeRender(VulkanCommandBuffer& vkb) override;
    void render(VulkanCommandBuffer& vkb, Scene& scene) override;

private:
    void renderGrids(VulkanCommandBuffer& vkb, Scene& scene);
    void renderModels(VulkanCommandBuffer& vkb, Scene& scene);
    void renderModelsInstanced(VulkanCommandBuffer& vkb, Scene& scene);

    RenderBufferPbr& buffer;
    RenderResources& resources;
    RenderPipelineGrid pipelineGrid;
    RenderPipelineModel pipelineModel;
    RenderPipelineModelInstanced pipelineModelInstanced;
    TexturePtr palette;
};
} // namespace Engine
