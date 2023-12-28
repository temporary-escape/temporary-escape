#pragma once

#include "RendererPlanetSurface.hpp"
#include "RendererSkybox.hpp"

namespace Engine {
class ENGINE_API RendererBackground {
public:
    explicit RendererBackground(const Config& config, VulkanRenderer& vulkan, RenderResources& resources,
                                VoxelShapeCache& voxelShapeCache);

    void render();
    void update(Scene& scene);

private:
    RendererSkybox rendererSkybox;
    RendererPlanetSurface rendererPlanetSurface;
};
} // namespace Engine
