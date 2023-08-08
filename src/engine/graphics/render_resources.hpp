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

    const VulkanBuffer& getSSAOKernel() const {
        return ssaoSamples.ubo;
    }

    const VulkanTexture& getSSAONoise() const {
        return ssaoSamples.noise;
    }

    const VulkanTexture& getDefaultSSAO() const {
        return defaultSSAO;
    }

    const VulkanTexture& getDefaultShadow() const {
        return defaultShadow;
    }

    const VulkanTexture& getDefaultBloom() const {
        return defaultBloom;
    }

private:
    void createSsaoNoise();
    void createSsaoSamples();

    VulkanRenderer& vulkan;
    Mesh meshFullScreenQuad;
    Mesh meshPlanet;
    Mesh meshSkyboxCube;
    SkyboxTextures defaultSkybox;
    struct {
        VulkanBuffer ubo;
        VulkanTexture noise;
    } ssaoSamples;
    VulkanTexture defaultSSAO;
    VulkanTexture defaultShadow;
    VulkanTexture defaultBloom;
};
} // namespace Engine
