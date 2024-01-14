#pragma once

#include "../../Assets/Texture.hpp"
#include "../../Scene/Scene.hpp"
#include "../Pipelines/RenderPipelineBullets.hpp"
#include "../Pipelines/RenderPipelineBulletsTrail.hpp"
#include "../Pipelines/RenderPipelineDebug.hpp"
#include "../Pipelines/RenderPipelineLines.hpp"
#include "../Pipelines/RenderPipelineParticles.hpp"
#include "../Pipelines/RenderPipelinePointCloud.hpp"
#include "../Pipelines/RenderPipelinePolyShape.hpp"
#include "../RenderBufferPbr.hpp"
#include "../RenderPass.hpp"

namespace Engine {
class ENGINE_API RenderPassForward : public RenderPass {
public:
    explicit RenderPassForward(const RenderOptions& options, VulkanRenderer& vulkan, RenderBufferPbr& buffer,
                               RenderResources& resources);

    void render(VulkanCommandBuffer& vkb, Scene& scene) override;

private:
    struct ForwardRenderJob {
        float order{0.0f};
        std::function<void()> fn;
    };

    template <typename T>
    void collectForRender(VulkanCommandBuffer& vkb, Scene& scene, std::vector<ForwardRenderJob>& jobs) {
        auto& camera = *scene.getPrimaryCamera();

        const auto& entities = scene.getView<ComponentTransform, T>(entt::exclude<TagDisabled>).each();
        for (auto&& [entity, transform, component] : entities) {
            const auto dist = camera.isOrthographic()
                                  ? -transform.getAbsolutePosition().y
                                  : glm::distance(transform.getAbsolutePosition(), camera.getEyesPos());

            jobs.emplace_back(ForwardRenderJob{
                dist,
                [&, t = &transform, c = &component] { renderSceneForward(vkb, scene, camera, *t, *c); },
            });
        }
    }

    void renderSceneForward(VulkanCommandBuffer& vkb, Scene& scene, const ComponentCamera& camera,
                            ComponentTransform& transform, ComponentPointCloud& component);
    void renderSceneForward(VulkanCommandBuffer& vkb, Scene& scene, const ComponentCamera& camera,
                            ComponentTransform& transform, ComponentPolyShape& component);
    void renderSceneForward(VulkanCommandBuffer& vkb, Scene& scene, const ComponentCamera& camera,
                            ComponentTransform& transform, ComponentLines& component);
    void renderSceneForward(VulkanCommandBuffer& vkb, Scene& scene, const ComponentCamera& camera,
                            ComponentTransform& transform, ComponentGrid& component);
    void renderSceneBullets(VulkanCommandBuffer& vkb, Scene& scene, const ComponentCamera& camera);
    void renderSceneBulletsTrail(VulkanCommandBuffer& vkb, Scene& scene, const ComponentCamera& camera);
    void renderSceneDebug(VulkanCommandBuffer& vkb, Scene& scene, const ComponentCamera& camera);

    VulkanRenderer& vulkan;
    RenderBufferPbr& buffer;
    RenderResources& resources;
    RenderPipeline* currentPipeline{nullptr};
    RenderPipelinePointCloud pipelinePointCloud;
    RenderPipelineLines pipelineLines;
    RenderPipelinePolyShape pipelinePolyShape;
    RenderPipelineBullets pipelineBullets;
    RenderPipelineBulletsTrail pipelineBulletsTrail;
    RenderPipelineParticles pipelineParticles;
    RenderPipelineDebug pipelineDebug;
};
} // namespace Engine
