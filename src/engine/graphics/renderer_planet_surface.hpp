#pragma once

#include "../assets/assets_manager.hpp"
#include "../scene/scene.hpp"
#include "render_buffer_planet.hpp"
#include "renderer_work.hpp"

namespace Engine {
class ENGINE_API Scene;

class ENGINE_API RendererPlanetSurface : public RendererWork {
public:
    explicit RendererPlanetSurface(const Config& config, const Vector2i& viewport, VulkanRenderer& vulkan,
                                   RenderResources& resources, AssetsManager& assetsManager,
                                   VoxelShapeCache& voxelShapeCache);

    void update(Scene& scene);

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
