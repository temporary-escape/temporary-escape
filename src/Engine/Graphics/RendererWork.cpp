#include "RendererWork.hpp"
#include "../Scene/Scene.hpp"

using namespace Engine;

RendererWork::RendererWork(const Config& config, VulkanRenderer& vulkan, VoxelShapeCache& voxelShapeCache) :
    Renderer{vulkan}, config{config}, vulkan{vulkan}, voxelShapeCache{voxelShapeCache} {
}

void RendererWork::render() {
    if (running && fence.isDone()) {
        jobsCurrent++;
        running = false;

        if (jobsCurrent >= jobsTotal) {
            finished();
        }
    }

    if (!running && jobsCurrent < jobsTotal) {
        fence = VulkanFence{vulkan};
        running = true;

        prepareScene();

        fence.reset();

        vulkan.dispose(std::move(vkb));
        vkb = vulkan.createCommandBuffer();

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkb.start(beginInfo);

        beforeRender(vkb, *scene, jobsCurrent);

        Renderer::render(vkb, *scene);

        postRender(vkb, *scene, jobsCurrent);

        vkb.end();
        vulkan.submitCommandBuffer(vkb, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, {}, {}, fence);
    }
}

bool RendererWork::isBusy() const {
    return running;
}

void RendererWork::startJobs(const size_t count) {
    jobsTotal = count;
    jobsCurrent = 0;
}

void RendererWork::prepareScene() {
    scene = std::make_unique<Scene>(config, &voxelShapeCache);

    auto entity = scene->createEntity();
    auto& transform = entity.addComponent<ComponentTransform>();
    auto& camera = entity.addComponent<ComponentCamera>(transform);

    camera.setProjection(75.0f);
    camera.lookAt({3.0f, 3.0f, 3.0f}, {0.0f, 0.0f, 0.0f});

    scene->setPrimaryCamera(entity);
}
