#pragma once

#include "../config.hpp"
#include "../scene/scene.hpp"
#include "../vulkan/vulkan_device.hpp"
#include "render_pass_skybox_color.hpp"
#include "render_pass_skybox_irradiance.hpp"
#include "render_pass_skybox_prefilter.hpp"
#include "skybox_textures.hpp"

namespace Engine {
class ENGINE_API SkyboxGenerator {
public:
    using Rng = std::mt19937_64;

    explicit SkyboxGenerator(const Config& config, VulkanRenderer& vulkan, RenderResources& resources,
                             AssetsManager& assetsManager);

    void enqueue(uint64_t seed, std::function<void(SkyboxTextures)> callback);
    void update(Scene& scene);
    void run();
    bool isBusy() const;

private:
    void startWork();
    void prepareCubemap();
    void prepareProperties();
    void prepareScene();
    void prepareNebulas(SkyboxGenerator::Rng& rng) const;
    void prepareStars(Rng& rng, const Vector2& size, size_t count);

    const Config& config;
    VulkanRenderer& vulkan;
    VulkanFence fence;
    VulkanCommandBuffer vkb;

    struct {
        uint64_t seed{0};
        bool isRunning{false};
        int side{0};
        bool postProcess{false};
        std::unique_ptr<Scene> scene;
        std::vector<ComponentCamera> cameras;
        std::function<void(SkyboxTextures)> callback;
    } work;

    struct {
        std::unique_ptr<RenderPassSkyboxColor> skyboxColor;
        std::unique_ptr<RenderPassSkyboxIrradiance> skyboxIrradiance;
        std::unique_ptr<RenderPassSkyboxPrefilter> skyboxPrefilter;
    } renderPasses;

    struct {
        TexturePtr star;
    } textures;

    SkyboxTextures skyboxTextures{};
};
} // namespace Engine
