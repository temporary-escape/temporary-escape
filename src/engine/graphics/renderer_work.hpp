#pragma once

#include "../assets/assets_manager.hpp"
#include "render_buffer_pbr.hpp"
#include "renderer.hpp"

namespace Engine {
class ENGINE_API Scene;

class ENGINE_API RendererWork : protected Renderer {
public:
    explicit RendererWork(const Config& config, VulkanRenderer& vulkan, VoxelShapeCache& voxelShapeCache);

    void render();
    bool isBusy() const;

protected:
    virtual void beforeRender(VulkanCommandBuffer& vkb, Scene& scene, size_t job) = 0;
    virtual void postRender(VulkanCommandBuffer& vkb, Scene& scene, size_t job) = 0;
    virtual void finished() = 0;
    void startJobs(size_t count);

private:
    void prepareScene();

    const Config& config;
    VulkanRenderer& vulkan;
    VoxelShapeCache& voxelShapeCache;
    VulkanFence fence;
    VulkanCommandBuffer vkb;
    bool running{false};
    size_t jobsTotal{0};
    size_t jobsCurrent{0};
    std::unique_ptr<Scene> scene;
};
} // namespace Engine
