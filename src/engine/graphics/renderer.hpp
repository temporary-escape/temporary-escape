#pragma once

#include "../config.hpp"
#include "../scene/scene.hpp"
#include "../vulkan/vulkan_renderer.hpp"
#include "skybox.hpp"

namespace Engine {
class ENGINE_API Renderer {
public:
    /*struct Pipelines {
        VulkanPipeline brdf;
        VulkanPipeline pbr;
        VulkanPipeline skybox;
        VulkanPipeline fxaa;
        VulkanPipeline bloomExtract;
        VulkanPipeline bloomBlur;
        VulkanPipeline bloomCombine;
        VulkanPipeline ssao;
        VulkanPipeline copy;
    };*/

    struct Options {
        float blurStrength{0.2f};
        float exposure{1.8f};
    };

    explicit Renderer(const Config& config, VulkanDevice& vulkan);
    ~Renderer();

    void update(const Vector2i& viewport);
    void begin();
    void end();
    void render(const Vector2i& viewport, Scene& scene, Skybox& skybox, const Options& options);
    void renderPassFront(bool clear);
    void present();

private:
    static constexpr size_t maxDirectionalLights = 4;

    struct DirectionalLightsUniform {
        Vector4 colors[maxDirectionalLights];
        Vector4 directions[maxDirectionalLights];
        int count{0};
    };

    struct GaussianWeightsUniform {
        Vector4 weight[32];
        int count{0};
    };

    struct SsaoSamplesUniform {
        Vector4 weights[64];
    };

    void createGaussianKernel(size_t size, double sigma);
    void createSsaoNoise();
    void createFullScreenQuad();
    void renderFullScreenQuad();
    void createSkyboxMesh();
    void renderSkyboxMesh();
    void createSsaoSamples();
    void updateDirectionalLightsUniform(Scene& scene);

    const Config& config;
    VulkanDevice& vulkan;
    // Pipelines& pipelines;

    struct {
        VulkanBuffer vbo;
        VulkanBuffer ibo;
        /// VulkanVertexInputFormat vboFormat;
    } fullScreenQuad;

    struct {
        VulkanBuffer vbo;
        VulkanBuffer ibo;
        // VulkanVertexInputFormat vboFormat;
    } skyboxMesh;

    struct {
        VulkanTexture texture;
        // VulkanFramebuffer fbo;
        Vector2i size{0, 0};
    } brdf;

    struct {
        VulkanBuffer ubo;
    } directionalLights;

    struct {
        VulkanBuffer ubo;
    } gaussianWeights;

    struct {
        VulkanBuffer ubo;
        VulkanTexture noise;
    } ssaoSamples;

    struct {
        Vector2i size{0, 0};
        VulkanTexture depth;

        VulkanTexture pbrColor;
        VulkanTexture pbrMetallicRoughnessAmbient;
        VulkanTexture pbrEmissive;
        VulkanTexture pbrNormal;
        VulkanFramebuffer pbrFbo;

        VulkanTexture ssaoColor;
        VulkanFramebuffer ssaoFbo;

        VulkanTexture forwardColor;
        VulkanFramebuffer forwardFbo;

        VulkanFramebuffer deferredFbo;

        // VulkanFramebuffer fxaa;
        // VulkanFramebuffer copy;

        VulkanTexture bloomColors[2];
        VulkanFramebuffer bloomFbos[2];

        VulkanTexture frontColor;
        VulkanFramebuffer frontFbo;
    } gBuffer;
};
} // namespace Engine
