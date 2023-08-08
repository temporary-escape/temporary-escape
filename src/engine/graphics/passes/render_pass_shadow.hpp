#pragma once

#include "../../assets/texture.hpp"
#include "../pipelines/render_pipeline_shadows_grid.hpp"
#include "../pipelines/render_pipeline_shadows_model.hpp"
#include "../pipelines/render_pipeline_shadows_model_instanced.hpp"
#include "../render_buffer_pbr.hpp"
#include "../render_pass.hpp"

namespace Engine {
class ENGINE_API RenderPassShadow : public RenderPass {
public:
    explicit RenderPassShadow(const RenderOptions& options, VulkanRenderer& vulkan, RenderBufferPbr& buffer,
                              RenderResources& resources, AssetsManager& assetsManager, uint32_t index);

    void beforeRender(VulkanCommandBuffer& vkb) override;
    void render(VulkanCommandBuffer& vkb, Scene& scene) override;

private:
    void renderGrids(VulkanCommandBuffer& vkb, Scene& scene);
    void renderModels(VulkanCommandBuffer& vkb, Scene& scene);
    void renderModelsInstanced(VulkanCommandBuffer& vkb, Scene& scene);

    RenderResources& resources;
    uint32_t index;
    RenderPipelineShadowsGrid pipelineGrid;
    RenderPipelineShadowsModel pipelineModel;
    RenderPipelineShadowsModelInstanced pipelineModelInstanced;
    TexturePtr palette;
};
} // namespace Engine
