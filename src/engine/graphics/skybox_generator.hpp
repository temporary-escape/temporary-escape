#pragma once

#include "../config.hpp"
#include "../scene/scene.hpp"
#include "../vulkan/vulkan_device.hpp"
#include "skybox.hpp"
#include <deque>

namespace Engine {
class ENGINE_API SkyboxGenerator {
public:
    explicit SkyboxGenerator(const Config& config, VulkanRenderer& vulkan, Registry& registry);

    Skybox generate(uint64_t seed);

private:
    using Rng = std::mt19937_64;

    struct Stars {
        VulkanBuffer vbo;
        size_t count{0};
    };

    struct CameraUniform {
        Matrix4 viewMatrix;
        Matrix4 projectionMatrix;
    };

    struct RenderPass {
        VulkanTexture texture;
        VulkanRenderPass renderPass;
        VulkanFramebuffer fbo;
    };

    struct Pipeline {
        VulkanDescriptorSetLayout descriptorSetLayout;
        VulkanPipeline pipeline;
    };

    struct SkyboxNebulaUniforms {
        Vector4 uColor;
        Vector4 uOffset;
        float uScale;
        float uIntensity;
        float uFalloff;
    };

    struct SkyboxPrefilterUniforms {
        float roughness{0};
    };

    struct StarVertex {
        Vector3 position;
        float brightness;
        Color4 color;
    };

    struct SkyboxStarsUniforms {
        Vector2 particleSize;
    };

    void createPipelineStars(Registry& registry, VulkanRenderPass& renderPass);
    void createPipelineNebula(Registry& registry, VulkanRenderPass& renderPass);
    void createPipelineIrradiance(Registry& registry, VulkanRenderPass& renderPass);
    void createPipelinePrefilter(Registry& registry, VulkanRenderPass& renderPass);
    void createAttachment(RenderPass& renderPass, const Vector2i& size, VkFormat format);
    void createFramebuffer(RenderPass& renderPass, const Vector2i& size);
    void createRenderPass(RenderPass& renderPass, const Vector2i& size, VkFormat format, VkImageLayout finalLayout,
                          VkAttachmentLoadOp loadOp);
    void createSkyboxMesh();
    void prepareStars(Rng& rng, Stars& stars, size_t count);
    static void prepareNebulaUbo(Rng& rng, std::vector<SkyboxNebulaUniforms>& params);
    void prepareCubemap(Skybox& skybox);
    void renderStars(VulkanCommandBuffer& vkb, Stars& stars, float particleSize);
    void renderNebula(VulkanCommandBuffer& vkb, const SkyboxNebulaUniforms& params);
    void renderIrradiance(VulkanCommandBuffer& vkb, VulkanTexture& color);
    void renderPrefilter(VulkanCommandBuffer& vkb, VulkanTexture& color, const Vector2i& viewport, int level);
    void prepareCameraUbo(int side);

    const Config& config;
    VulkanRenderer& vulkan;

    struct {
        VulkanBuffer vbo;
    } mesh;

    struct {
        Pipeline stars;
        Pipeline nebula;
        Pipeline irradiance;
        Pipeline prefilter;
    } shaders;

    struct {
        RenderPass color;
        RenderPass irradiance;
        RenderPass prefilter;
        VulkanBuffer cameraUbo;
    } renderPasses;
};
} // namespace Engine
