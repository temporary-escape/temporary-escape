#pragma once

#include "../vulkan/vulkan_renderer.hpp"
#include "mesh.hpp"

namespace Engine {
class RenderResources {
public:
    explicit RenderResources(VulkanRenderer& vulkan);

    const Mesh& getMeshFullScreenQuad() const {
        return meshFullScreenQuad;
    }

    const Mesh& getMeshPlanet() const {
        return meshPlanet;
    }

    const Mesh& getMeshSkyboxCube() const {
        return meshSkyboxCube;
    }

private:
    Mesh meshFullScreenQuad;
    Mesh meshPlanet;
    Mesh meshSkyboxCube;
};
} // namespace Engine
