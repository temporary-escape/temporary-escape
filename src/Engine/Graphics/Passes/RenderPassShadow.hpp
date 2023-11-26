#pragma once

#include "../../Assets/Texture.hpp"
#include "../Pipelines/RenderPipelineShadowsGrid.hpp"
#include "../Pipelines/RenderPipelineShadowsModel.hpp"
#include "../Pipelines/RenderPipelineShadowsModelInstanced.hpp"
#include "../Pipelines/RenderPipelineShadowsModelSkinned.hpp"
#include "../RenderBufferPbr.hpp"
#include "../RenderPass.hpp"

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
    void renderModelsSkinned(VulkanCommandBuffer& vkb, Scene& scene);
    void renderModelsInstanced(VulkanCommandBuffer& vkb, Scene& scene);

    const RenderOptions& options;
    RenderResources& resources;
    uint32_t index;
    RenderPipelineShadowsGrid pipelineGrid;
    RenderPipelineShadowsModel pipelineModel;
    RenderPipelineShadowsModelSkinned pipelineModelSkinned;
    RenderPipelineShadowsModelInstanced pipelineModelInstanced;
    TexturePtr palette;
};
} // namespace Engine
