#pragma once

#include "../config.hpp"
#include "../scene/scene.hpp"
#include "../vulkan/vulkan_renderer.hpp"
#include "shader.hpp"
#include "shaders/shader_brdf.hpp"
#include "shaders/shader_component_grid.hpp"
#include "shaders/shader_pass_bloom_blur.hpp"
#include "shaders/shader_pass_bloom_combine.hpp"
#include "shaders/shader_pass_bloom_extract.hpp"
#include "shaders/shader_pass_debug.hpp"
#include "shaders/shader_pass_fxaa.hpp"
#include "shaders/shader_pass_pbr.hpp"
#include "shaders/shader_pass_skybox.hpp"
#include "shaders/shader_pass_ssao.hpp"
#include "skybox.hpp"

namespace Engine {
class ENGINE_API VoxelShapeCache;

class ENGINE_API Renderer {
public:
    struct Options {
        float blurStrength{0.2f};
        float exposure{1.8f};
    };

    explicit Renderer(const Config& config, VulkanRenderer& vulkan, ShaderModules& shaderModules,
                      VoxelShapeCache& voxelShapeCache);
    ~Renderer();

    void render(const Vector2i& viewport, Scene& scene, Skybox& skybox, const Options& options);

    VulkanRenderer& getVulkan() {
        return vulkan;
    }

private:
    struct RenderPassMesh {
        VulkanBuffer vbo;
        VulkanBuffer ibo;
        VkIndexType indexType{VK_INDEX_TYPE_UINT16};
        uint32_t count{0};
    };

    struct RenderPass {
        VulkanFramebuffer fbo;
        VulkanRenderPass renderPass;
        VulkanSemaphore semaphore;
    };

    void createFullScreenQuad();
    void createSkyboxMesh();
    void createRenderPasses();
    void createRenderPassBrdf();
    void createRenderPassPbr();
    void createRenderPassSsao();
    void createRenderPassForward();
    void createRenderPassFxaa();
    void createRenderPassBloomExtract();
    void createRenderPassBloomBlur();
    void createShaders(ShaderModules& shaderModules);
    void createDepthTexture();
    void createSsaoNoise();
    void createSsaoSamples();
    void createGaussianKernel(size_t size, double sigma);
    void createAttachment(VulkanTexture& texture, const Vector2i& size, VkFormat format, VkImageUsageFlags usage,
                          VkImageAspectFlags aspectMask);
    void renderMesh(VulkanCommandBuffer& vkb, RenderPassMesh& mesh);
    void renderBrdf();
    void renderPassPbr(const Vector2i& viewport, Scene& scene, const Options& options);
    void renderPassSsao(const Vector2i& viewport, Scene& scene, const Options& options);
    void renderPassForward(const Vector2i& viewport, Scene& scene, Skybox& skybox, const Options& options);
    void renderPassFxaa(const Vector2i& viewport);
    void renderPassBloomExtract();
    void renderPassBloomBlur();
    void renderPassBloomBlur(VulkanCommandBuffer& vkb, size_t idx, bool vertical);
    void renderPassBloomCombine(VulkanCommandBuffer& vkb);
    void renderSceneGrids(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene);
    void renderSceneSkybox(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene, Skybox& skybox);
    void renderLightingPbr(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene, Skybox& skybox);
    void transitionDepthForWrite();
    void transitionForWrite(VulkanCommandBuffer& vkb, const size_t idx);
    VkFormat findDepthFormat();
    void updateDirectionalLights(Scene& scene);

    // static constexpr size_t maxDirectionalLights = 4;

    /*struct DirectionalLightsUniform {
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
    };*/

    /*void createGaussianKernel(size_t size, double sigma);
    void createSsaoNoise();
    void createFullScreenQuad();
    void renderFullScreenQuad();
    void createSkyboxMesh();
    void renderSkyboxMesh();
    void createSsaoSamples();
    void updateDirectionalLightsUniform(Scene& scene);*/

    const Config& config;
    VulkanRenderer& vulkan;
    VoxelShapeCache& voxelShapeCache;
    Vector2i lastViewportSize;
    Vector2i bloomViewportSize;

    struct {
        ShaderBrdf brdf;
        ShaderComponentGrid componentGrid;
        ShaderPassSSAO passSsao;
        ShaderPassDebug passDebug;
        ShaderPassSkybox passSkybox;
        ShaderPassPbr passPbr;
        ShaderPassFXAA passFxaa;
        ShaderPassBloomExtract passBloomExtract;
        ShaderPassBloomBlur passBloomBlur[2];
        ShaderPassBloomCombine passBloomCombine;
    } shaders;

    struct {
        VkFormat depthFormat;
        VulkanTexture brdf;
        VulkanTexture depth;
        VulkanTexture pbrAlbedoAmbient;
        VulkanTexture pbrEmissiveRoughness;
        VulkanTexture pbrNormalMetallic;
        VulkanTexture ssao;
        VulkanTexture forward;
        VulkanTexture aux;
        VulkanTexture blurred[2];
    } textures;

    struct {
        RenderPass brdf;
        RenderPass pbr;
        RenderPass ssao;
        RenderPass forward;
        RenderPass fxaa;
        RenderPass bloomExtract;
        RenderPass bloomBlur[2];
    } renderPasses;

    struct {
        RenderPassMesh fullScreenQuad;
        RenderPassMesh skybox;
    } meshes;

    struct {
        VulkanBuffer ubo;
        VulkanTexture noise;
    } ssaoSamples;

    struct {
        VulkanBuffer ubo;
    } blurWeights;

    struct {
        VulkanDoubleBuffer ubo;
    } directionalLights;

    // Pipelines& pipelines;

    /*struct {
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
    } gBuffer;*/
};
} // namespace Engine
