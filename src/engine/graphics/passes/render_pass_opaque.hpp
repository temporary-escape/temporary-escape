#pragma once

#include "../../assets/texture.hpp"
#include "../pipelines/render_pipeline_grid.hpp"
#include "../pipelines/render_pipeline_model.hpp"
#include "../pipelines/render_pipeline_model_instanced.hpp"
#include "../pipelines/render_pipeline_model_skinned.hpp"
#include "../render_buffer_pbr.hpp"
#include "../render_pass.hpp"

namespace Engine {
class ENGINE_API RenderPassOpaque : public RenderPass {
public:
    explicit RenderPassOpaque(const RenderOptions& options, VulkanRenderer& vulkan, RenderBufferPbr& buffer,
                              RenderResources& resources, AssetsManager& assetsManager);

    void beforeRender(VulkanCommandBuffer& vkb) override;
    void render(VulkanCommandBuffer& vkb, Scene& scene) override;
    void afterRender(VulkanCommandBuffer& vkb) override;
    void setMousePos(const Vector2i& value) {
        mousePos = value;
    }
    uint32_t getMousePosEntity() const;

private:
    void renderGrids(VulkanCommandBuffer& vkb, Scene& scene);
    void renderModels(VulkanCommandBuffer& vkb, Scene& scene);
    void renderModelsSkinned(VulkanCommandBuffer& vkb, Scene& scene);
    void renderModelsInstanced(VulkanCommandBuffer& vkb, Scene& scene);

    RenderBufferPbr& buffer;
    RenderResources& resources;
    RenderPipelineGrid pipelineGrid;
    RenderPipelineModel pipelineModel;
    RenderPipelineModelSkinned pipelineModelSkinned;
    RenderPipelineModelInstanced pipelineModelInstanced;
    TexturePtr palette;
    Vector2i mousePos;
    VulkanDoubleBuffer entityColorBuffer;
};
} // namespace Engine
