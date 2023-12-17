#pragma once

#include "../Utils/Random.hpp"
#include "../Vulkan/VulkanRenderer.hpp"
#include "Mesh.hpp"
#include "SkyboxTextures.hpp"

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

    const Mesh& getMeshBullet() const {
        return meshBullet;
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

    const VulkanTexture& getPalette() const {
        return palette;
    }

    const VulkanTexture& getBrdf() const {
        return brdf;
    }

    const VulkanTexture& getSkyboxStar() const {
        return skyboxStar;
    }

private:
    void createSsaoNoise();
    void createSsaoSamples();
    void createPalette();
    void createBrdf();
    void createSkyboxStar();

    VulkanRenderer& vulkan;
    Mesh meshFullScreenQuad;
    Mesh meshPlanet;
    Mesh meshSkyboxCube;
    Mesh meshBullet;
    SkyboxTextures defaultSkybox;
    struct {
        VulkanBuffer ubo;
        VulkanTexture noise;
    } ssaoSamples;
    VulkanTexture defaultSSAO;
    VulkanTexture defaultShadow;
    VulkanTexture defaultBloom;
    VulkanTexture palette;
    VulkanTexture brdf;
    VulkanTexture skyboxStar;
};
} // namespace Engine
