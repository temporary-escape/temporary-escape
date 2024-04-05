#pragma once

#include "../../Assets/Texture.hpp"
#include "../Pipelines/RenderPipelineGrid.hpp"
#include "../Pipelines/RenderPipelineModel.hpp"
#include "../Pipelines/RenderPipelineModelInstanced.hpp"
#include "../Pipelines/RenderPipelineModelSkinned.hpp"
#include "../Pipelines/RenderPipelinePlanet.hpp"
#include "../RenderBufferPbr.hpp"
#include "../RenderPass.hpp"

namespace Engine {
class ENGINE_API RenderPassOpaque : public RenderPass {
public:
    explicit RenderPassOpaque(const RenderOptions& options, VulkanRenderer& vulkan, RenderBufferPbr& buffer,
                              RenderResources& resources);

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
    void renderPlanets(VulkanCommandBuffer& vkb, Scene& scene);
    void renderModelsSkinned(VulkanCommandBuffer& vkb, Scene& scene);
    void renderModelsInstanced(VulkanCommandBuffer& vkb, Scene& scene);

    RenderBufferPbr& buffer;
    RenderResources& resources;
    RenderPipelineGrid pipelineGrid;
    RenderPipelineModel pipelineModel;
    RenderPipelineModelSkinned pipelineModelSkinned;
    RenderPipelineModelInstanced pipelineModelInstanced;
    RenderPipelinePlanet pipelinePlanet;
    Vector2i mousePos;
    VulkanDoubleBuffer entityColorBuffer;
};
} // namespace Engine
