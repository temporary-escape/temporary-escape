#pragma once

#include "../config.hpp"
#include "../scene/scene.hpp"
#include "../vulkan/vulkan_device.hpp"
#include "planet_textures.hpp"
#include "render_resources.hpp"

namespace Engine {
class ENGINE_API PlanetGenerator {
public:
    explicit PlanetGenerator(const Vector2i& textureSize, VulkanRenderer& vulkan, RenderResources& resources,
                             AssetsManager& assetsManager);

    PlanetTextures render(uint64_t seed, const PlanetTypePtr& planetType);
    void enqueue(uint64_t seed, const PlanetTypePtr& planetType, std::function<void(PlanetTextures)>&& callback);
    void update(Scene& scene);
    void run();
    bool isBusy() const;

private:
    using Rng = std::mt19937_64;

    void startWork();
    void prepareCubemap();

    VulkanRenderer& vulkan;
    Vector2i textureSize;
    VulkanFence fence;
    VulkanCommandBuffer vkb;

    struct {
        uint64_t seed{0};
        bool isRunning{false};
        int side{0};
        PlanetTypePtr planetType;
        std::function<void(PlanetTextures)> callback;
    } work;

    struct {
        // std::unique_ptr<RenderPassPlanetSurface> planetSurface;
        // std::unique_ptr<RenderPassPlanetNormal> planetNormal;
    } renderPasses;

    PlanetTextures planetTextures{};
};
} // namespace Engine
