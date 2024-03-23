#pragma once

#include "../Utils/Random.hpp"
#include "../Vulkan/VulkanRenderer.hpp"
#include "MaterialTextures.hpp"
#include "Mesh.hpp"
#include "SkyboxTextures.hpp"

namespace Engine {
class ENGINE_API RenderResources {
public:
    explicit RenderResources(VulkanRenderer& vulkan, const VulkanBuffer& blockMaterials,
                             const MaterialTextures& materialTextures);
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

    const Mesh& getMeshOrbit() const {
        return meshOrbit;
    }

    const Mesh& getMeshLineForward() const {
        return meshLineForward;
    }

    const Mesh& getMeshSpaceDust() const {
        return meshSpaceDust;
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

    const VulkanTexture& getSpaceDust() const {
        return spaceDust;
    }

    const VulkanBuffer& getBlockMaterials() const {
        return blockMaterials;
    }

    const MaterialTextures& getMaterialTextures() const {
        return materialTextures;
    }

private:
    void createSsaoNoise();
    void createSsaoSamples();
    void createPalette();
    void createBrdf();
    void createSkyboxStar();
    void createSpaceDust();

    VulkanRenderer& vulkan;
    const VulkanBuffer& blockMaterials;
    const MaterialTextures& materialTextures;
    Mesh meshFullScreenQuad;
    Mesh meshPlanet;
    Mesh meshSkyboxCube;
    Mesh meshBullet;
    Mesh meshOrbit;
    Mesh meshSpaceDust;
    Mesh meshLineForward;
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
    VulkanTexture spaceDust;
};
} // namespace Engine
