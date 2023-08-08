#include "renderer.hpp"
#include "../scene/scene.hpp"
#include "../utils/exceptions.hpp"

using namespace Engine;

Renderer::Renderer(const RenderOptions& options, VulkanRenderer& vulkan) : options{options}, vulkan{vulkan} {
}

void Renderer::render(VulkanCommandBuffer& vkb, Scene& scene, const Vector2i& viewport) {
    const auto camera = scene.getPrimaryCamera();
    if (!camera) {
        EXCEPTION("Failed to render scene error: no primary camera");
    }

    camera->recalculate(vulkan, viewport);

    scene.recalculate(vulkan);

    for (auto& pass : passes) {
        try {
            pass->begin(vkb);
            pass->render(vkb, scene);
            pass->end(vkb);
        } catch (...) {
            EXCEPTION_NESTED("Failed to render scene pass: {}", pass->getName());
        }
    }
}

void Renderer::blit(VulkanCommandBuffer& vkb) {
}

void Renderer::addRenderPass(std::unique_ptr<RenderPass> pass) {
    passes.push_back(std::move(pass));
}

void Renderer::create() {
    for (const auto& pass : passes) {
        try {
            pass->create();
        } catch (...) {
            EXCEPTION_NESTED("Failed to create render pass: {}", pass->getName());
        }
    }
}
