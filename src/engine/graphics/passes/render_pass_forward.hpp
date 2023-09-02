#pragma once

#include "../../assets/texture.hpp"
#include "../../scene/scene.hpp"
#include "../pipelines/render_pipeline_bullets.hpp"
#include "../pipelines/render_pipeline_bullets_trail.hpp"
#include "../pipelines/render_pipeline_debug.hpp"
#include "../pipelines/render_pipeline_lines.hpp"
#include "../pipelines/render_pipeline_particles.hpp"
#include "../pipelines/render_pipeline_point_cloud.hpp"
#include "../pipelines/render_pipeline_poly_shape.hpp"
#include "../render_buffer_pbr.hpp"
#include "../render_pass.hpp"

namespace Engine {
class ENGINE_API RenderPassForward : public RenderPass {
public:
    explicit RenderPassForward(const RenderOptions& options, VulkanRenderer& vulkan, RenderBufferPbr& buffer,
                               RenderResources& resources, AssetsManager& assetsManager);

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
                [&, t = &transform, c = &component] { renderSceneForward(vkb, camera, *t, *c); },
            });
        }
    }

    void renderSceneForward(VulkanCommandBuffer& vkb, const ComponentCamera& camera, ComponentTransform& transform,
                            ComponentPointCloud& component);
    void renderSceneForward(VulkanCommandBuffer& vkb, const ComponentCamera& camera, ComponentTransform& transform,
                            ComponentPolyShape& component);
    void renderSceneForward(VulkanCommandBuffer& vkb, const ComponentCamera& camera, ComponentTransform& transform,
                            ComponentLines& component);
    void renderSceneForward(VulkanCommandBuffer& vkb, const ComponentCamera& camera, ComponentTransform& transform,
                            ComponentGrid& component);
    void renderSceneBullets(VulkanCommandBuffer& vkb, const ComponentCamera& camera, Scene& scene);
    void renderSceneBulletsTrail(VulkanCommandBuffer& vkb, const ComponentCamera& camera, Scene& scene);
    void renderSceneDebug(VulkanCommandBuffer& vkb, const ComponentCamera& camera, Scene& scene);

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
