#pragma once

#include "../Config.hpp"
#include "../Vulkan/VulkanDevice.hpp"
#include "View.hpp"

namespace Engine {
class ENGINE_API Renderer {
public:
    struct Pipelines {
        VulkanPipeline pbr;
        VulkanPipeline skybox;
    };

    explicit Renderer(const Config& config, VulkanDevice& vulkan, Pipelines& pipelines);
    ~Renderer();

    void update(const Vector2i& viewport);
    void begin();
    void end();
    void render(const Vector2i& viewport, View& view);
    void present();

private:
    void createFullScreenQuad();
    void renderFullScreenQuad();
    void createSkyboxMesh();
    void renderSkyboxMesh();

    const Config& config;
    VulkanDevice& vulkan;
    Pipelines& pipelines;

    struct {
        VulkanBuffer vbo;
        VulkanBuffer ibo;
        VulkanVertexInputFormat vboFormat;
    } fullScreenQuad;

    struct {
        VulkanBuffer vbo;
        VulkanBuffer ibo;
        VulkanVertexInputFormat vboFormat;
    } skyboxMesh;

    struct {
        Vector2i size{0, 0};
        VulkanTexture color;
        VulkanTexture pbrColor;
        VulkanTexture pbrMetallicRoughnessAmbient;
        VulkanTexture pbrEmissive;
        VulkanTexture pbrNormal;
        VulkanTexture pbrResult;
        VulkanTexture depth;
    } gBuffer;

    VulkanFramebuffer fbo;
};
} // namespace Engine
