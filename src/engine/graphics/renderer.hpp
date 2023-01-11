#pragma once

#include "../config.hpp"
#include "../scene/scene.hpp"
#include "../vulkan/vulkan_renderer.hpp"
#include "shader.hpp"
#include "shaders/shader_brdf.hpp"
#include "shaders/shader_component_debug.hpp"
#include "shaders/shader_component_grid.hpp"
#include "shaders/shader_component_point_cloud.hpp"
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
        bool bloomEnabled{true};
    };

    explicit Renderer(const Config& config, VulkanRenderer& vulkan, ShaderModules& shaderModules,
                      VoxelShapeCache& voxelShapeCache);
    ~Renderer();

    void render(const Vector2i& viewport, Scene& scene, Skybox& skybox, const Options& options);

    VulkanRenderer& getVulkan() {
        return vulkan;
    }

private:
    using RenderPassMesh = Mesh;

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
    void createRenderPassLighting();
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
    void renderPassLighting(const Vector2i& viewport, Scene& scene, Skybox& skybox, const Options& options);
    void renderPassForward(const Vector2i& viewport, Scene& scene, Skybox& skybox, const Options& options);
    void renderPassFxaa(const Vector2i& viewport);
    void renderPassBloomExtract();
    void renderPassBloomBlur();
    void renderPassBloomBlur(VulkanCommandBuffer& vkb, size_t idx, bool vertical);
    void renderPassBloomCombine(VulkanCommandBuffer& vkb, const Options& options);
    void renderSceneGrids(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene);
    void renderSceneForwards(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene);
    void renderSceneForwardDebug(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene);
    void renderSceneSkybox(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene, Skybox& skybox);
    void renderLightingPbr(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene, Skybox& skybox);
    void transitionDepthForReadOnlyOptimal();
    void transitionForWrite(VulkanCommandBuffer& vkb, const size_t idx);
    VkFormat findDepthFormat();
    void updateDirectionalLights(Scene& scene);
    void renderMesh(VulkanCommandBuffer& vkb, const Mesh& mesh);

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
        ShaderComponentPointCloud componentPointCloud;
        ShaderComponentDebug componentDebug;
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
        RenderPass lighting;
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
};
} // namespace Engine
