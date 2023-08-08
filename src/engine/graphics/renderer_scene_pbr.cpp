#include "renderer_scene_pbr.hpp"
#include "../utils/exceptions.hpp"
#include "passes/render_pass_skybox.hpp"

using namespace Engine;

RendererScenePbr::RendererScenePbr(const RenderOptions& options, VulkanRenderer& vulkan, RenderResources& resources,
                                   AssetsManager& assetsManager) :
    Renderer{options, vulkan}, renderBufferPbr{options, vulkan} {

    try {
        addRenderPass(std::make_unique<RenderPassSkybox>(vulkan, renderBufferPbr, resources, assetsManager));
    } catch (...) {
        EXCEPTION_NESTED("Failed to setup render passes");
    }
    create();
}
