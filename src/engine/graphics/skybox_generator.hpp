#pragma once

#include "../config.hpp"
#include "../scene/scene.hpp"
#include "../vulkan/vulkan_device.hpp"
#include "shaders/shader_skybox_irradiance.hpp"
#include "shaders/shader_skybox_nebula.hpp"
#include "shaders/shader_skybox_prefilter.hpp"
#include "shaders/shader_skybox_stars.hpp"
#include "skybox.hpp"
#include <deque>

namespace Engine {
class ENGINE_API SkyboxGenerator {
public:
    explicit SkyboxGenerator(const Config& config, VulkanRenderer& vulkan, ShaderModules& shaderModules);

    Skybox generate(uint64_t seed);

private:
    using Rng = std::mt19937_64;

    struct Stars {
        VulkanBuffer vbo;
        size_t count{0};
    };

    struct RenderPass {
        VulkanTexture texture;
        VulkanRenderPass renderPass;
        VulkanFramebuffer fbo;
    };

    void createAttachment(RenderPass& renderPass, const Vector2i& size, VkFormat format);
    void createFramebuffer(RenderPass& renderPass, const Vector2i& size);
    void createRenderPass(RenderPass& renderPass, const Vector2i& size, VkFormat format, VkImageLayout finalLayout,
                          VkAttachmentLoadOp loadOp);
    void createSkyboxMesh();
    void prepareStars(Rng& rng, Stars& stars, size_t count);
    void prepareNebulaUbo(Rng& rng, std::vector<ShaderSkyboxNebula::Uniforms>& params);
    void prepareCubemap(Skybox& skybox);
    void renderStars(VulkanCommandBuffer& vkb, Stars& stars, int side, float particleSize);
    void renderNebula(VulkanCommandBuffer& vkb, const ShaderSkyboxNebula::Uniforms& params);
    void renderIrradiance(VulkanCommandBuffer& vkb, VulkanTexture& color);
    void renderPrefilter(VulkanCommandBuffer& vkb, VulkanTexture& color, const Vector2i& viewport, int level);
    void prepareCameraUbo(int side);
    void transitionTexture(VulkanCommandBuffer& vkb, VulkanTexture& texture, VkImageLayout oldLayout,
                           VkImageLayout newLayout, VkPipelineStageFlags srcStageMask,
                           VkPipelineStageFlags dstStageMask);
    void copyTexture(VulkanCommandBuffer& vkb, VulkanTexture& source, VulkanTexture& target, int side);

    const Config& config;
    VulkanRenderer& vulkan;

    struct {
        VulkanBuffer vbo;
    } skybox;

    struct {
        ShaderSkyboxStars stars;
        ShaderSkyboxNebula nebula;
        ShaderSkyboxIrradiance irradiance;
        ShaderSkyboxPrefilter prefilter;
    } shaders;

    struct {
        RenderPass color;
        RenderPass irradiance;
        RenderPass prefilter;
        VulkanBuffer cameraUbo;
    } renderPasses;
};
} // namespace Engine
