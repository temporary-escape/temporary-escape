#pragma once

#include "../Assets/AssetsManager.hpp"
#include "../Scene/Scene.hpp"
#include "RenderBufferPlanet.hpp"
#include "RendererWork.hpp"

namespace Engine {
class ENGINE_API Scene;

class ENGINE_API RendererPlanetSurface : public RendererWork {
public:
    explicit RendererPlanetSurface(const Config& config, const Vector2i& viewport, VulkanRenderer& vulkan,
                                   RenderResources& resources, VoxelShapeCache& voxelShapeCache);

    void update(Scene& scene);
    PlanetTextures renderPlanet(uint64_t seed, const PlanetTypePtr& planetType);

protected:
    void beforeRender(VulkanCommandBuffer& vkb, Scene& scene, size_t job) override;
    void postRender(VulkanCommandBuffer& vkb, Scene& scene, size_t job) override;
    void finished() override;
    void copyTexture(VulkanCommandBuffer& vkb, uint32_t attachment, const VulkanTexture& target, int side);
    void prepareTexture(VulkanCommandBuffer& vkb, const VulkanTexture& target);
    void prepareCubemap(VulkanCommandBuffer& vkb);

private:
    VulkanRenderer& vulkan;
    Vector2i viewport;
    RenderBufferPlanet renderBufferPlanet;

    struct {
        Entity entity;
        uint64_t seed{0};
        PlanetTypePtr planetType{nullptr};
    } work;

    PlanetTextures planetTextures;
};
} // namespace Engine
