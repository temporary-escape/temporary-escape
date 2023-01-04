#pragma once

#include "../config.hpp"
#include "../scene/scene.hpp"
#include "../vulkan/vulkan_renderer.hpp"
#include "shaders/shader_brdf.hpp"
#include "shaders/shader_component_grid.hpp"
#include "shaders/shader_pass_debug.hpp"
#include "shaders/shader_pass_ssao.hpp"
#include "skybox.hpp"

namespace Engine {
class ENGINE_API VoxelShapeCache;

class ENGINE_API Renderer {
public:
    using ShaderLoadQueue = std::list<std::function<void(const Config&, VulkanRenderer&)>>;

    struct Shaders {
        ShaderBrdf brdf;
        ShaderComponentGrid componentGrid;
        ShaderPassSSAO passSsao;
        ShaderPassDebug passDebug;

        ShaderLoadQueue createLoadQueue();
    };
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

    explicit Renderer(const Config& config, VulkanRenderer& vulkan, Shaders& shaders, VoxelShapeCache& voxelShapeCache);
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
        std::vector<VulkanTexture> textures;
        VulkanFramebuffer fbo;
        VulkanRenderPass renderPass;
        VulkanSemaphore semaphore;
    };

    void createFullScreenQuad();
    void createRenderPasses();
    void createRenderPassBrdf();
    void createRenderPassPbr();
    void createRenderPassSsao();
    void finalizeShaders();
    void createDepthTexture();
    void createSsaoNoise();
    void createSsaoSamples();
    void createGaussianKernel(size_t size, double sigma);
    void createAttachment(VulkanTexture& texture, const Vector2i& size, VkFormat format, VkImageUsageFlags usage,
                          VkImageAspectFlags aspectMask);
    void renderMesh(VulkanCommandBuffer& vkb, RenderPassMesh& mesh);
    void renderBrdf();
    void renderPassPbr(const Vector2i& viewport, Scene& scene, const Options& options);
    void renderPassSSAO(const Vector2i& viewport, Scene& scene, const Options& options);
    void renderSceneGrids(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene);
    void transitionDepthForWrite();
    void transitionDepthForRead();
    VkFormat findDepthFormat();

    struct GaussianWeightsUniform {
        Vector4 weight[32];
        int count{0};
    };

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
    Shaders& shaders;
    VoxelShapeCache& voxelShapeCache;
    Vector2i lastViewportSize;

    struct {
        VulkanTexture depth;
        VkFormat depthFormat;
        RenderPass brdf;
        RenderPass pbr;
        RenderPass ssao;
    } renderPasses;

    struct {
        RenderPassMesh fullScreenQuad;
    } meshes;

    struct {
        VulkanBuffer ubo;
        VulkanTexture noise;
    } ssaoSamples;

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
