#pragma once

#include "../utils/random.hpp"
#include "../vulkan/vulkan_renderer.hpp"
#include "mesh.hpp"
#include "skybox_textures.hpp"

namespace Engine {
class ENGINE_API RenderResources {
public:
    explicit RenderResources(VulkanRenderer& vulkan);
    ~RenderResources();

    const Mesh& getMeshFullScreenQuad() const {
        return meshFullScreenQuad;
    }

    const Mesh& getMeshPlanet() const {
        return meshPlanet;
    }

    const Mesh& getMeshSkyboxCube() const {
        return meshSkyboxCube;
    }

    const SkyboxTextures& getDefaultSkybox() const {
        return defaultSkybox;
    }

private:
    VulkanRenderer& vulkan;
    Mesh meshFullScreenQuad;
    Mesh meshPlanet;
    Mesh meshSkyboxCube;
    SkyboxTextures defaultSkybox;
};
} // namespace Engine
