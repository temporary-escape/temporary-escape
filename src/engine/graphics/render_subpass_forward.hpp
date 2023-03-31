#pragma once

#include "render_pipeline.hpp"
#include "render_subpass.hpp"

namespace Engine {
class ENGINE_API RenderSubpassForward : public RenderSubpass {
public:
    explicit RenderSubpassForward(VulkanRenderer& vulkan, Registry& registry);
    virtual ~RenderSubpassForward() = default;

    void render(VulkanCommandBuffer& vkb, Scene& scene);

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
                            ComponentDebug& component);

    VulkanRenderer& vulkan;
    RenderPipeline pipelineDebug;
    Mesh cube;
    RenderPipeline* currentPipeline{nullptr};
};
} // namespace Engine