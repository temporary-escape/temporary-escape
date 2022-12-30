#pragma once

#include "../config.hpp"
#include "../scene/scene.hpp"
#include "../vulkan/vulkan_device.hpp"
#include "skybox.hpp"
#include <deque>

namespace Engine {
class ENGINE_API SkyboxGenerator {
public:
    /*struct Pipelines {
        VulkanPipeline stars;
        VulkanPipeline nebula;
        VulkanPipeline irradiance;
        VulkanPipeline prefilter;
    };*/

    explicit SkyboxGenerator(const Config& config, VulkanRenderer& vulkan);

    void updateSeed(uint64_t seed);
    void generate(uint64_t seed);
    void render();
    [[nodiscard]] bool isReady() const;
    [[nodiscard]] Skybox& get() {
        return skybox;
    }
    [[nodiscard]] const Skybox& get() const {
        return skybox;
    }

private:
    struct Stars {
        VulkanBuffer vbo;
        // VulkanVertexInputFormat vboFormat;
        size_t count{0};
    };

    void prepareFbo();
    void prepareCubemap();
    void prepareNebulaMesh();
    void prepareNebulaUbo();
    void renderStars(Stars& stars, int side, float particleSize, bool clear);
    void renderNebula(VulkanBuffer& ubo, int side);
    void prepareStars(Stars& stars, size_t count);
    void blit(VulkanTexture& source, VulkanTexture& target, int side, int width);
    void blitLevel(VulkanTexture& source, VulkanTexture& target, int side, int width, int level);
    void renderIrradiance(int side);
    void renderPrefilter(int side, int level);

    const Config& config;
    VulkanDevice& vulkan;
    Skybox skybox;
    std::optional<uint64_t> seed;

    std::deque<std::function<void()>> jobs;
    std::mt19937_64 rng;

    struct {
        VulkanBuffer vbo;
        // VulkanVertexInputFormat vboFormat;
        std::list<VulkanBuffer> ubos;
        size_t count{0};
    } nebula;

    Stars starsSmall;
    Stars starsLarge;

    struct {
        VulkanTexture texture;
        VulkanFramebuffer fbo;
    } main;

    struct {
        VulkanTexture texture;
        VulkanFramebuffer fbo;
    } irradiance;

    struct {
        VulkanTexture texture;
        VulkanFramebuffer fbo;
    } prefilter;
};
} // namespace Engine
