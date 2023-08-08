#pragma once

#include "../assets/assets_manager.hpp"
#include "../scene/scene.hpp"
#include "render_buffer_skybox.hpp"
#include "renderer_work.hpp"

namespace Engine {
class ENGINE_API Scene;

class ENGINE_API RendererSkybox : public RendererWork {
public:
    explicit RendererSkybox(const Config& config, VulkanRenderer& vulkan, RenderResources& resources,
                            AssetsManager& assetsManager);

    void update(Scene& scene);

protected:
    void beforeRender(VulkanCommandBuffer& vkb, Scene& scene, size_t job) override;
    void postRender(VulkanCommandBuffer& vkb, Scene& scene, size_t job) override;
    void finished() override;
    void copyTexture(VulkanCommandBuffer& vkb, uint32_t attachment, const VulkanTexture& target, int side);
    void prepareTexture(VulkanCommandBuffer& vkb, const VulkanTexture& target);
    void prepareCubemap(VulkanCommandBuffer& vkb);

private:
    using Rng = std::mt19937_64;

    void prepareProperties(Scene& scene);
    void prepareNebulas(Scene& scene, Rng& rng) const;
    void prepareStars(Scene& scene, Rng& rng, const Vector2& size, size_t count);

    const Config& config;
    VulkanRenderer& vulkan;
    RenderBufferSkybox renderBufferSkybox;

    struct {
        Entity entity;
        uint64_t seed{0};
    } work;

    struct {
        TexturePtr star;
    } textures;

    SkyboxTextures skyboxTextures;
};
} // namespace Engine
