#include "RendererBackground.hpp"

using namespace Engine;

RendererBackground::RendererBackground(const Config& config, VulkanRenderer& vulkan, RenderResources& resources,
                                       VoxelShapeCache& voxelShapeCache) :
    rendererSkybox{config, vulkan, resources, voxelShapeCache},
    rendererPlanetSurface{config, Vector2i{config.graphics.planetTextureSize}, vulkan, resources, voxelShapeCache} {
}

void RendererBackground::render() {
    rendererSkybox.render();
    rendererPlanetSurface.render();
}

void RendererBackground::update(Scene& scene) {
    if (!rendererPlanetSurface.isBusy()) {
        rendererSkybox.update(scene);
    }

    if (!rendererSkybox.isBusy()) {
        rendererPlanetSurface.update(scene);
    }
}
