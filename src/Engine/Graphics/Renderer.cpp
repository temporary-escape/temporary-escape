#include "Renderer.hpp"
#include "../Scene/Scene.hpp"
#include "../Utils/Exceptions.hpp"

using namespace Engine;

Renderer::Renderer(VulkanRenderer& vulkan) : vulkan{vulkan} {
}

void Renderer::render(VulkanCommandBuffer& vkb, VulkanCommandBuffer& vkbc, Scene& scene) {
    scene.recalculate(vulkan);

    for (auto& pass : passes) {
        if (pass->isExcluded()) {
            continue;
        }

        try {
            auto& cmd = pass->isCompute() ? vkbc : vkb;
            pass->begin(cmd);
            pass->render(cmd, scene);
            pass->end(cmd);
        } catch (...) {
            EXCEPTION_NESTED("Failed to render scene pass: {}", pass->getName());
        }
    }
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
