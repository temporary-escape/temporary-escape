#pragma once

#include "../config.hpp"
#include "../font/font_family.hpp"
#include "../scene/scene.hpp"
#include "../vulkan/vulkan_renderer.hpp"
#include "canvas.hpp"
#include "nuklear.hpp"
#include "shader.hpp"
#include "shaders/shader_brdf.hpp"
#include "shaders/shader_component_2d_shape.hpp"
#include "shaders/shader_component_debug.hpp"
#include "shaders/shader_component_grid.hpp"
#include "shaders/shader_component_planet_surface.hpp"
#include "shaders/shader_component_point_cloud.hpp"
#include "shaders/shader_component_poly_shape.hpp"
#include "shaders/shader_component_world_text.hpp"
#include "shaders/shader_pass_bloom_blur.hpp"
#include "shaders/shader_pass_bloom_combine.hpp"
#include "shaders/shader_pass_bloom_extract.hpp"
#include "shaders/shader_pass_debug.hpp"
#include "shaders/shader_pass_fxaa.hpp"
#include "shaders/shader_pass_pbr.hpp"
#include "shaders/shader_pass_skybox.hpp"
#include "shaders/shader_pass_ssao.hpp"
#include "shaders/shader_position_feedback.hpp"
#include "skybox.hpp"

namespace Engine {
class ENGINE_API VoxelShapeCache;

class ENGINE_API Renderer {
public:
    struct Options {
        bool bloomEnabled{true};
        std::optional<float> exposureOverride;
    };

    explicit Renderer(const Config& config, const Vector2i& viewport, VulkanRenderer& vulkan, Canvas& canvas,
                      Nuklear& nuklear, ShaderModules& shaderModules, VoxelShapeCache& voxelShapeCache,
                      VoxelPalette& voxelPalette, FontFamily& font);
    virtual ~Renderer();

    void render(const Vector2i& viewport, Scene& scene, const Skybox& skybox, const Options& options,
                NuklearWindow& nuklearWindow);

    VulkanRenderer& getVulkan() {
        return vulkan;
    }

protected:
    virtual VulkanFramebuffer& getParentFramebuffer() {
        return vulkan.getSwapChainFramebuffer();
    }

    virtual VulkanRenderPass& getParentRenderPass() {
        return vulkan.getRenderPass();
    }

    virtual VulkanFenceOpt getParentInFlightFence() {
        return vulkan.getCurrentInFlightFence();
    }

    virtual VulkanSemaphoreOpt getParentImageAvailableSemaphore() {
        return vulkan.getCurrentImageAvailableSemaphore();
    }

    virtual VulkanSemaphoreOpt getParentRenderFinishedSemaphore() {
        return vulkan.getCurrentRenderFinishedSemaphore();
    }

private:
    using RenderPassMesh = Mesh;

    struct RenderPass {
        VulkanFramebuffer fbo;
        VulkanRenderPass renderPass;
        VulkanSemaphore semaphore;
    };

    struct ForwardRenderJob {
        float order{0.0f};
        std::function<void()> fn;
    };

    void createFullScreenQuad();
    void createSkyboxMesh();
    void createPlanetMesh();
    void createCircleMesh();
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
    void renderPassCompute(const Vector2i& viewport, Scene& scene, const Options& options);
    void renderPassPbr(const Vector2i& viewport, Scene& scene, const Options& options);
    void renderPassSsao(const Vector2i& viewport, Scene& scene, const Options& options);
    void renderPassLighting(const Vector2i& viewport, Scene& scene, const Skybox& skybox, const Options& options);
    void renderPassForward(const Vector2i& viewport, Scene& scene, const Skybox& skybox, const Options& options);
    void renderPassFxaa(const Vector2i& viewport);
    void renderPassBloomExtract();
    void renderPassBloomBlur();
    void renderPassBloomBlur(VulkanCommandBuffer& vkb, size_t idx, bool vertical);
    void renderPassBloomCombine(VulkanCommandBuffer& vkb, const Options& options);
    void renderSceneGrids(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene);
    void renderSceneForward(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene);
    void renderSceneForwardNonHDR(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene);
    void renderSceneCanvas(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene);
    void renderNuklear(VulkanCommandBuffer& vkb, const Vector2i& viewport, NuklearWindow& nuklearWindow);

    template <typename T>
    void collectForRender(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene,
                          std::vector<ForwardRenderJob>& jobs) {
        auto& camera = *scene.getPrimaryCamera();

        const auto& entities = scene.getView<ComponentTransform, T>(entt::exclude<TagDisabled>).each();
        for (auto&& [entity, transform, component] : entities) {
            const auto dist = camera.isOrthographic()
                                  ? -transform.getAbsolutePosition().y
                                  : glm::distance(transform.getAbsolutePosition(), camera.getEyesPos());

            jobs.emplace_back(ForwardRenderJob{
                dist,
                [&, t = &transform, c = &component] { renderSceneForward(vkb, camera, *t, *c); },
            });
        }
    }

    void renderSceneForward(VulkanCommandBuffer& vkb, const ComponentCamera& camera, ComponentTransform& transform,
                            ComponentDebug& component);
    void renderSceneForward(VulkanCommandBuffer& vkb, const ComponentCamera& camera, ComponentTransform& transform,
                            ComponentIconPointCloud& component);
    void renderSceneForward(VulkanCommandBuffer& vkb, const ComponentCamera& camera, ComponentTransform& transform,
                            ComponentPointCloud& component);
    void renderSceneForward(VulkanCommandBuffer& vkb, const ComponentCamera& camera, ComponentTransform& transform,
                            ComponentLines& component);
    void renderSceneForward(VulkanCommandBuffer& vkb, const ComponentCamera& camera, ComponentTransform& transform,
                            ComponentPolyShape& component);
    void renderSceneForward(VulkanCommandBuffer& vkb, const ComponentCamera& camera, ComponentTransform& transform,
                            ComponentWorldText& component);
    void renderSceneForward(VulkanCommandBuffer& vkb, const ComponentCamera& camera, ComponentTransform& transform,
                            ComponentPlanet& component);
    void renderSceneForward(VulkanCommandBuffer& vkb, const ComponentCamera& camera, ComponentTransform& transform,
                            Component2DShape& component);
    void renderSceneSkybox(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene, const Skybox& skybox);
    void renderLightingPbr(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene, const Skybox& skybox);
    void transitionDepthForReadOnlyOptimal();
    void transitionForWrite(VulkanCommandBuffer& vkb, size_t idx);
    VkFormat findDepthFormat();
    void updateDirectionalLights(Scene& scene);
    void renderMesh(VulkanCommandBuffer& vkb, const Mesh& mesh);

    const Config& config;
    VulkanRenderer& vulkan;
    Canvas& canvas;
    Nuklear& nuklear;
    VoxelShapeCache& voxelShapeCache;
    VoxelPalette& voxelPalette;
    FontFamily& font;
    Vector2i lastViewportSize;
    Vector2i bloomViewportSize;

    struct {
        VulkanSemaphore semaphore;
    } compute;

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
        ShaderComponentLines componentLines;
        ShaderPositionFeedback positionFeedback;
        ShaderComponentPolyShape componentPolyShape;
        ShaderComponentWorldText componentWorldText;
        ShaderComponentPlanetSurface componentPlanetSurface;
        ShaderComponent2DShape component2DShape;
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
        RenderPassMesh planet;
        RenderPassMesh circle;
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

    Shader* currentForwardShader{nullptr};
};
} // namespace Engine
