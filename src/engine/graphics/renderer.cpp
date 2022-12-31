#include "renderer.hpp"
#include "../utils/exceptions.hpp"

#define CMP "Renderer"

using namespace Engine;

Renderer::ShaderLoadQueue Renderer::Shaders::createLoadQueue() {
    return {
        [this](const Config& config, VulkanRenderer& renderer) {
            brdf = ShaderBrdf{config, renderer};
        },
    };
}

Renderer::Renderer(const Config& config, VulkanDevice& vulkan) : config{config}, vulkan{vulkan} {
    createGaussianKernel(15, 6.5);
    createSsaoNoise();
    createSsaoSamples();
    createFullScreenQuad();
    createSkyboxMesh();
}

Renderer::~Renderer() {
}

void Renderer::update(const Vector2i& viewport) {
    /*if (gBuffer.size != viewport && viewport.x != 0 && viewport.y != 0) {
        gBuffer.size = viewport;

        const auto swapchainFormat = vulkan.getSwapchainFormat();

        auto desc = VulkanTexture::Descriptor{};
        desc.size = viewport;
        desc.format = swapchainFormat;
        desc.levels = 1;
        desc.type = VulkanTexture::Type::VK_IMAGE_TYPE_2D;
        desc.usage = VulkanTexture::Usage::VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                     VulkanTexture::Usage::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                     VulkanTexture::Usage::VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        gBuffer.frontColor = vulkan.createTexture(desc);

        desc = VulkanTexture::Descriptor{};
        desc.size = viewport;
        desc.format = VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
        desc.levels = 1;
        desc.type = VulkanTexture::Type::VK_IMAGE_TYPE_2D;
        desc.usage = VulkanTexture::Usage::VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                     VulkanTexture::Usage::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                     VulkanTexture::Usage::VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        gBuffer.pbrColor = vulkan.createTexture(desc);

        desc = VulkanTexture::Descriptor{};
        desc.size = viewport;
        desc.format = VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
        desc.levels = 1;
        desc.type = VulkanTexture::Type::VK_IMAGE_TYPE_2D;
        desc.usage = VulkanTexture::Usage::VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                     VulkanTexture::Usage::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                     VulkanTexture::Usage::VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        gBuffer.pbrMetallicRoughnessAmbient = vulkan.createTexture(desc);

        desc = VulkanTexture::Descriptor{};
        desc.size = viewport;
        desc.format = VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
        desc.levels = 1;
        desc.type = VulkanTexture::Type::VK_IMAGE_TYPE_2D;
        desc.usage = VulkanTexture::Usage::VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                     VulkanTexture::Usage::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                     VulkanTexture::Usage::VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        gBuffer.pbrEmissive = vulkan.createTexture(desc);

        desc = VulkanTexture::Descriptor{};
        desc.size = viewport;
        desc.format = VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
        desc.levels = 1;
        desc.type = VulkanTexture::Type::VK_IMAGE_TYPE_2D;
        desc.usage = VulkanTexture::Usage::VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                     VulkanTexture::Usage::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                     VulkanTexture::Usage::VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        gBuffer.pbrNormal = vulkan.createTexture(desc);

        desc = VulkanTexture::Descriptor{};
        desc.size = viewport;
        desc.format = VkFormat::VK_FORMAT_R16G16B16A16_SFLOAT;
        desc.levels = 1;
        desc.type = VulkanTexture::Type::VK_IMAGE_TYPE_2D;
        desc.usage = VulkanTexture::Usage::VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                     VulkanTexture::Usage::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                     VulkanTexture::Usage::VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        gBuffer.forwardColor = vulkan.createTexture(desc);

        desc = VulkanTexture::Descriptor{};
        desc.size = viewport;
        desc.format = VkFormat::VK_FORMAT_R16G16B16A16_SFLOAT;
        desc.levels = 1;
        desc.type = VulkanTexture::Type::VK_IMAGE_TYPE_2D;
        desc.usage = VulkanTexture::Usage::VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                     VulkanTexture::Usage::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                     VulkanTexture::Usage::VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        gBuffer.bloomColors[0] = vulkan.createTexture(desc);
        gBuffer.bloomColors[1] = vulkan.createTexture(desc);

        desc = VulkanTexture::Descriptor{};
        desc.size = viewport;
        desc.format = VulkanTexture::Format::VK_FORMAT_D32_SFLOAT;
        desc.levels = 1;
        desc.type = VulkanTexture::Type::VK_IMAGE_TYPE_2D;
        desc.usage = VulkanTexture::Usage::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                     VulkanTexture::Usage::VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        gBuffer.depth = vulkan.createTexture(desc);

        desc = VulkanTexture::Descriptor{};
        desc.size = viewport;
        desc.format = VulkanTexture::Format::VK_FORMAT_R8_UNORM;
        desc.levels = 1;
        desc.type = VulkanTexture::Type::VK_IMAGE_TYPE_2D;
        desc.usage = VulkanTexture::Usage::VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                     VulkanTexture::Usage::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                     VulkanTexture::Usage::VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        gBuffer.ssaoColor = vulkan.createTexture(desc);

        std::vector<VulkanFramebufferAttachment> attachments = {
            {gBuffer.pbrColor},  {gBuffer.pbrEmissive}, {gBuffer.pbrMetallicRoughnessAmbient},
            {gBuffer.pbrNormal}, {gBuffer.depth},
        };

        gBuffer.pbrFbo = vulkan.createFramebuffer(viewport, attachments);

        attachments = {
            {gBuffer.forwardColor},
        };

        gBuffer.deferredFbo = vulkan.createFramebuffer(viewport, attachments);

        attachments = {
            {gBuffer.bloomColors[1]},
        };

        gBuffer.bloomFbos[1] = vulkan.createFramebuffer(viewport, attachments);

        attachments = {
            {gBuffer.forwardColor},
            {gBuffer.depth},
        };

        gBuffer.forwardFbo = vulkan.createFramebuffer(viewport, attachments);

        attachments = {
            {gBuffer.bloomColors[0]},
        };

        gBuffer.bloomFbos[0] = vulkan.createFramebuffer(viewport, attachments);

        attachments = {
            {gBuffer.bloomColors[1]},
        };

        gBuffer.bloomFbos[1] = vulkan.createFramebuffer(viewport, attachments);

        attachments = {
            {gBuffer.frontColor},
        };

        gBuffer.frontFbo = vulkan.createFramebuffer(viewport, attachments);

        attachments = {
            {gBuffer.ssaoColor},
        };

        gBuffer.ssaoFbo = vulkan.createFramebuffer(viewport, attachments);

        vulkan.deviceWaitIdle();
        Log::d(CMP, "Renderer framebuffer created of size {}", gBuffer.size);
    }

    if (!brdf.texture && pipelines.brdf) {
        brdf.size = {config.brdfSize, config.brdfSize};

        auto desc = VulkanTexture::Descriptor{};
        desc.size = brdf.size;
        desc.format = VkFormat::VK_FORMAT_R16G16B16A16_SFLOAT;
        desc.levels = 1;
        desc.type = VulkanTexture::Type::VK_IMAGE_TYPE_2D;
        desc.usage = VulkanTexture::Usage::VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                     VulkanTexture::Usage::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                     VulkanTexture::Usage::VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        brdf.texture = vulkan.createTexture(desc);

        std::vector<VulkanFramebufferAttachment> attachments = {
            {brdf.texture},
        };

        brdf.fbo = vulkan.createFramebuffer(brdf.size, attachments);
        vulkan.deviceWaitIdle();

        VulkanFramebufferAttachmentReference colorAttachment{};
        colorAttachment.clearValue.color = {0.0f, 0.0f, 0.0f, 0.0f};

        vulkan.beginRenderPass(brdf.fbo, {colorAttachment});

        vulkan.setViewport({0, 0}, brdf.size);
        vulkan.setScissor({0, 0}, brdf.size);
        vulkan.setViewportState();

        vulkan.bindPipeline(pipelines.brdf);
        renderFullScreenQuad();

        vulkan.endRenderPass();
        vulkan.deviceWaitIdle();

        Log::d(CMP, "BRDF texture created of size: {}", brdf.size);
    }*/
}

void Renderer::begin() {
    /*vulkan.setViewport({0, 0}, gBuffer.size);
    vulkan.setScissor({0, 0}, gBuffer.size);
    vulkan.setViewportState();*/
}

void Renderer::end() {
    // vulkan.endRenderPass();
}

void Renderer::render(const Vector2i& viewport, Scene& scene, Skybox& skybox, const Options& options) {
    /*// ======================================== PBR scene ========================================
    VulkanFramebufferAttachmentReference pbrColorAttachment{};
    pbrColorAttachment.clearValue.color = {0.0f, 0.0f, 0.0f, 0.0f};

    VulkanFramebufferAttachmentReference pbrEmissiveAttachment{};
    pbrEmissiveAttachment.clearValue.color = {0.0f, 0.0f, 0.0f, 0.0f};

    VulkanFramebufferAttachmentReference pbrMetallicRoughnessAmbientAttachment{};
    pbrMetallicRoughnessAmbientAttachment.clearValue.color = {0.0f, 0.0f, 0.0f, 0.0f};

    VulkanFramebufferAttachmentReference pbrNormalAttachment{};
    pbrNormalAttachment.clearValue.color = {0.0f, 0.0f, 0.0f, 0.0f};

    VulkanFramebufferAttachmentReference depthStencilAttachment{};
    depthStencilAttachment.clearValue.depthStencil.depth = 1.0f;
    depthStencilAttachment.clearValue.depthStencil.stencil = 0;

    vulkan.beginRenderPass(gBuffer.pbrFbo, {
                                               pbrColorAttachment,
                                               pbrEmissiveAttachment,
                                               pbrMetallicRoughnessAmbientAttachment,
                                               pbrNormalAttachment,
                                               depthStencilAttachment,
                                           });

    updateDirectionalLightsUniform(scene);

    vulkan.setRasterization(VkPolygonMode::VK_POLYGON_MODE_FILL, VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT,
                            VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);

    try {
        scene.renderPbr(vulkan, viewport);
    } catch (...) {
        EXCEPTION_NESTED("Something went wrong during renderPbr()");
    }

    vulkan.endRenderPass();

    vulkan.setRasterization(VkPolygonMode::VK_POLYGON_MODE_FILL, VkCullModeFlagBits::VK_CULL_MODE_NONE,
                            VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);

    const auto camera = scene.getPrimaryCamera();

    // ======================================== Skybox ========================================

    VulkanFramebufferAttachmentReference forwardColorAttachment{};
    forwardColorAttachment.clearValue.color = {0.0f, 0.0f, 0.0f, 0.0f};
    forwardColorAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;

    VulkanFramebufferAttachmentReference forwardDepthStencilAttachment{};
    forwardDepthStencilAttachment.clearValue.depthStencil.depth = 1.0f;
    forwardDepthStencilAttachment.clearValue.depthStencil.stencil = 0;
    forwardDepthStencilAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;

    vulkan.beginRenderPass(gBuffer.forwardFbo, {forwardColorAttachment, forwardDepthStencilAttachment});

    vulkan.bindPipeline(pipelines.skybox);
    vulkan.bindUniformBuffer(camera->getUboZeroPos(), 0);
    vulkan.bindTexture(skybox.getTexture(), 1);
    renderSkyboxMesh();

    vulkan.endRenderPass();

    // ======================================== SSAO ========================================

    VulkanFramebufferAttachmentReference ssaoAttachment{};
    ssaoAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;

    vulkan.beginRenderPass(gBuffer.ssaoFbo, {ssaoAttachment});

    vulkan.bindPipeline(pipelines.ssao);
    vulkan.bindUniformBuffer(camera->getUbo(), 0);
    vulkan.bindUniformBuffer(ssaoSamples.ubo, 1);
    vulkan.bindTexture(gBuffer.depth, 2);
    vulkan.bindTexture(gBuffer.pbrNormal, 3);
    vulkan.bindTexture(ssaoSamples.noise, 4);
    vulkan.pushConstant(0, Vector2{viewport} / 4.0f);
    renderFullScreenQuad();

    vulkan.endRenderPass();

    // ======================================== Deferred ========================================

    VulkanFramebufferAttachmentReference deferredResultAttachment{};
    deferredResultAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;
    deferredResultAttachment.clearValue.color = {0.0f, 0.0f, 0.0f, 0.0f};

    vulkan.beginRenderPass(gBuffer.deferredFbo, {deferredResultAttachment});

    vulkan.bindPipeline(pipelines.pbr);
    vulkan.bindUniformBuffer(camera->getUbo(), 0);
    vulkan.bindUniformBuffer(directionalLights.ubo, 1);
    vulkan.bindTexture(gBuffer.pbrColor, 2);
    vulkan.bindTexture(gBuffer.pbrEmissive, 3);
    vulkan.bindTexture(gBuffer.pbrMetallicRoughnessAmbient, 4);
    vulkan.bindTexture(gBuffer.pbrNormal, 5);
    vulkan.bindTexture(skybox.getIrradiance(), 6);
    vulkan.bindTexture(skybox.getPrefilter(), 7);
    vulkan.bindTexture(brdf.texture, 8);
    vulkan.bindTexture(gBuffer.depth, 9);
    vulkan.bindTexture(gBuffer.ssaoColor, 10);
    renderFullScreenQuad();

    vulkan.endRenderPass();

    // ======================================== Forward ========================================
    deferredResultAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;

    vulkan.beginRenderPass(gBuffer.forwardFbo, {deferredResultAttachment, forwardDepthStencilAttachment});

    try {
        scene.renderFwd(vulkan, viewport);
    } catch (...) {
        EXCEPTION_NESTED("Something went wrong during renderFwd()");
    }

    vulkan.endRenderPass();

    // ======================================== Bloom extract ========================================
    auto tempColorAttachment = VulkanFramebufferAttachmentReference{};
    tempColorAttachment.clearValue.color = {0.0f, 0.0f, 0.0f, 0.0f};
    tempColorAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;

    vulkan.beginRenderPass(gBuffer.bloomFbos[0], {tempColorAttachment});

    vulkan.bindPipeline(pipelines.bloomExtract);
    vulkan.bindTexture(gBuffer.forwardColor, 0);
    renderFullScreenQuad();

    vulkan.endRenderPass();

    // ======================================== Bloom blur ========================================
    for (auto i = 0; i < 6; i++) {
        tempColorAttachment = VulkanFramebufferAttachmentReference{};
        tempColorAttachment.clearValue.color = {0.0f, 0.0f, 0.0f, 0.0f};
        tempColorAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;

        vulkan.beginRenderPass(gBuffer.bloomFbos[(i + 1) % 2], {tempColorAttachment});

        vulkan.bindPipeline(pipelines.bloomBlur);
        vulkan.bindUniformBuffer(gaussianWeights.ubo, 0);
        vulkan.pushConstant(0, i < 3);
        vulkan.bindTexture(gBuffer.bloomColors[i % 2], 1);
        renderFullScreenQuad();

        vulkan.endRenderPass();
    }

    // ======================================== Bloom combine ========================================
    tempColorAttachment = VulkanFramebufferAttachmentReference{};
    tempColorAttachment.clearValue.color = {0.0f, 0.0f, 0.0f, 0.0f};
    tempColorAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;

    vulkan.beginRenderPass(gBuffer.bloomFbos[0], {tempColorAttachment});

    vulkan.bindPipeline(pipelines.bloomCombine);
    vulkan.pushConstant(0, options.blurStrength);
    vulkan.pushConstant(sizeof(float), options.exposure);
    vulkan.bindTexture(gBuffer.forwardColor, 0);
    vulkan.bindTexture(gBuffer.bloomColors[1], 1);
    renderFullScreenQuad();

    vulkan.endRenderPass();

    // ======================================== FXAA ========================================
    VulkanFramebufferAttachmentReference frontColorAttachment{};
    frontColorAttachment.clearValue.color = {0.0f, 0.0f, 0.0f, 0.0f};
    tempColorAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;

    vulkan.beginRenderPass(gBuffer.frontFbo, {tempColorAttachment});

    vulkan.bindPipeline(pipelines.fxaa);
    vulkan.pushConstant(0, Vector2{viewport});
    vulkan.bindTexture(gBuffer.bloomColors[0], 0);
    renderFullScreenQuad();*/

    /*try {
        scene.renderFwd(vulkan, viewport);
    } catch (...) {
        EXCEPTION_NESTED("Something went wrong during renderFwd()");
    }*/
}

void Renderer::renderPassFront(bool clear) {
    /*VulkanFramebufferAttachmentReference frontColorAttachment{};
    frontColorAttachment.clearValue.color = {0.0f, 0.0f, 0.0f, 0.0f};
    if (clear) {
        frontColorAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
    } else {
        frontColorAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;
    }

    vulkan.beginRenderPass(gBuffer.frontFbo, {frontColorAttachment});*/
}

void Renderer::present() {
    // vulkan.submitPresentQueue(gBuffer.frontColor);
}

void Renderer::createGaussianKernel(const size_t size, double sigma) {
    /*const auto weights = gaussianKernel((size - 1) * 2 + 1, sigma);
    GaussianWeightsUniform data;

    for (size_t i = 0; i < size; i++) {
        const auto w = static_cast<float>(weights[size - i - 1]);
        data.weight[i] = Vector4{w, w, w, 1.0f};
    }
    data.count = static_cast<int>(size);

    gaussianWeights.ubo =
        vulkan.createBuffer(VulkanBuffer::Type::Uniform, VulkanBuffer::Usage::Static, sizeof(GaussianWeightsUniform));
    gaussianWeights.ubo.subData(&data, 0, sizeof(GaussianWeightsUniform));*/
}

void Renderer::createSsaoNoise() {
    /*std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::random_device rd;
    std::default_random_engine generator{rd()};

    std::vector<glm::vec3> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++) {
        glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0,
                        0.0f); // rotate around z-axis (in tangent space)
        ssaoNoise.push_back(noise);
    }

    VulkanTexture::Descriptor desc{};
    desc.size = Vector2i{4, 4};
    desc.format = VulkanTexture::Format::VK_FORMAT_R32G32B32_SFLOAT;
    desc.levels = 1;
    desc.type = VulkanTexture::Type::VK_IMAGE_TYPE_2D;
    desc.usage =
        VulkanTexture::Usage::VK_IMAGE_USAGE_SAMPLED_BIT | VulkanTexture::Usage::VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    ssaoSamples.noise = vulkan.createTexture(desc);
    ssaoSamples.noise.subData(0, {0, 0}, desc.size, ssaoNoise.data());*/
}

void Renderer::createFullScreenQuad() {
    /*static const std::vector<Vector2> vertices = {
        {-1.0f, -1.0f},
        {1.0f, -1.0f},
        {1.0f, 1.0f},
        {-1.0f, 1.0f},
    };

    static const std::vector<uint32_t> indices = {
        0, 1, 2, 2, 3, 0,
    };

    fullScreenQuad.vbo = vulkan.createBuffer(VulkanBuffer::Type::Vertex, VulkanBuffer::Usage::Dynamic,
                                             sizeof(Vector2) * vertices.size());
    fullScreenQuad.vbo.subData(vertices.data(), 0, sizeof(Vector2) * vertices.size());

    fullScreenQuad.ibo =
        vulkan.createBuffer(VulkanBuffer::Type::Index, VulkanBuffer::Usage::Dynamic, sizeof(uint32_t) * indices.size());
    fullScreenQuad.ibo.subData(indices.data(), 0, sizeof(uint32_t) * indices.size());

    fullScreenQuad.vboFormat = vulkan.createVertexInputFormat({
        {
            0,
            {
                {0, 0, VulkanVertexInputFormat::Format::Vec2},
            },
        },
    });*/
}

void Renderer::renderFullScreenQuad() {
    /*vulkan.setDepthStencilState(false, false);
    VulkanBlendState blendState{};
    blendState.blendEnable = false;
    blendState.colorBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
    blendState.alphaBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
    blendState.srcColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE;
    blendState.dstColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendState.srcAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE;
    blendState.dstAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendState.colorWriteMask = VkColorComponentFlagBits::VK_COLOR_COMPONENT_FLAG_BITS_MAX_ENUM;
    vulkan.setBlendState({blendState});

    vulkan.bindVertexBuffer(fullScreenQuad.vbo, 0);
    vulkan.bindVertexInputFormat(fullScreenQuad.vboFormat);
    vulkan.bindIndexBuffer(fullScreenQuad.ibo, 0, VK_INDEX_TYPE_UINT32);
    vulkan.setInputAssembly(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    vulkan.drawIndexed(6, 1, 0, 0, 0);*/
}

void Renderer::createSkyboxMesh() {
    /*static const std::vector<uint32_t> indices = {
        0,  1,  2,  2,  3,  0,  4,  5,  6,  6,  7,  4,  8,  9,  10, 10, 11, 8,
        12, 13, 14, 14, 15, 12, 16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 21, 23,
    };

    static const std::vector<Vector3> vertices = {
        {-1.0f, 1.0f, -1.0f},  // 0
        {-1.0f, -1.0f, -1.0f}, // 1
        {1.0f, -1.0f, -1.0f},  // 2
        {1.0f, 1.0f, -1.0f},   // 3

        {-1.0f, -1.0f, 1.0f},  // 4
        {-1.0f, -1.0f, -1.0f}, // 5
        {-1.0f, 1.0f, -1.0f},  // 6
        {-1.0f, 1.0f, 1.0f},   // 7

        {1.0f, -1.0f, -1.0f}, // 8
        {1.0f, -1.0f, 1.0f},  // 9
        {1.0f, 1.0f, 1.0f},   // 10
        {1.0f, 1.0f, -1.0f},  // 11

        {-1.0f, -1.0f, 1.0f}, // 12
        {-1.0f, 1.0f, 1.0f},  // 13
        {1.0f, 1.0f, 1.0f},   // 14
        {1.0f, -1.0f, 1.0f},  // 15

        {-1.0f, 1.0f, -1.0f}, // 16
        {1.0f, 1.0f, -1.0f},  // 17
        {1.0f, 1.0f, 1.0f},   // 18
        {-1.0f, 1.0f, 1.0f},  // 19

        {-1.0f, -1.0f, -1.0f}, // 20
        {-1.0f, -1.0f, 1.0f},  // 21
        {1.0f, -1.0f, -1.0f},  // 22
        {1.0f, -1.0f, 1.0f},   // 23
    };

    skyboxMesh.vbo = vulkan.createBuffer(VulkanBuffer::Type::Vertex, VulkanBuffer::Usage::Dynamic,
                                         sizeof(Vector3) * vertices.size());
    skyboxMesh.vbo.subData(vertices.data(), 0, sizeof(Vector3) * vertices.size());

    skyboxMesh.ibo =
        vulkan.createBuffer(VulkanBuffer::Type::Index, VulkanBuffer::Usage::Dynamic, sizeof(uint32_t) * indices.size());
    skyboxMesh.ibo.subData(indices.data(), 0, sizeof(uint32_t) * indices.size());

    skyboxMesh.vboFormat = vulkan.createVertexInputFormat({
        {
            0,
            {
                {0, 0, VulkanVertexInputFormat::Format::Vec3},
            },
        },
    });*/
}

void Renderer::renderSkyboxMesh() {
    /*vulkan.setDepthStencilState(false, true);
    VulkanBlendState blendState{};
    blendState.blendEnable = false;
    blendState.colorBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
    blendState.alphaBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
    blendState.srcColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE;
    blendState.dstColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendState.srcAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE;
    blendState.dstAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendState.colorWriteMask = VkColorComponentFlagBits::VK_COLOR_COMPONENT_FLAG_BITS_MAX_ENUM;
    vulkan.setBlendState({blendState});

    Matrix4 transform = glm::scale(Vector3{500.0f, 500.0f, 500.0f});
    vulkan.pushConstant(0, transform);
    vulkan.bindVertexBuffer(skyboxMesh.vbo, 0);
    vulkan.bindVertexInputFormat(skyboxMesh.vboFormat);
    vulkan.bindIndexBuffer(skyboxMesh.ibo, 0, VK_INDEX_TYPE_UINT32);
    vulkan.setInputAssembly(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    vulkan.drawIndexed(6 * 6, 1, 0, 0, 0);*/
}

void Renderer::updateDirectionalLightsUniform(Scene& scene) {
    /*DirectionalLightsUniform uniform{};

    auto& system = scene.getComponentSystem<ComponentDirectionalLight>();
    for (const auto& component : system) {
        uniform.colors[uniform.count] = component->getColor();
        uniform.directions[uniform.count] = Vector4{component->getObject().getPosition(), 0.0f};

        uniform.count++;
        if (uniform.count >= maxDirectionalLights) {
            break;
        }
    }

    if (!directionalLights.ubo) {
        directionalLights.ubo = vulkan.createBuffer(VulkanBuffer::Type::Uniform, VulkanBuffer::Usage::Dynamic,
                                                    sizeof(DirectionalLightsUniform));
        directionalLights.ubo.subData(&uniform, 0, sizeof(DirectionalLightsUniform));
    } else {
        auto dst = directionalLights.ubo.mapPtr(sizeof(DirectionalLightsUniform));
        std::memcpy(dst, &uniform, sizeof(DirectionalLightsUniform));
        directionalLights.ubo.unmap();
    }*/
}

void Renderer::createSsaoSamples() {
    /*std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::random_device rd;
    std::default_random_engine generator{rd()};

    SsaoSamplesUniform uniform{};

    for (unsigned int i = 0; i < sizeof(SsaoSamplesUniform::weights) / sizeof(Vector4); ++i) {
        Vector3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0,
                       randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i) / 64.0f;

        // scale samples s.t. they're more aligned to center of kernel
        scale = ::lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        uniform.weights[i] = Vector4{sample, 1.0f};
    }

    ssaoSamples.ubo =
        vulkan.createBuffer(VulkanBuffer::Type::Uniform, VulkanBuffer::Usage::Dynamic, sizeof(SsaoSamplesUniform));
    ssaoSamples.ubo.subData(&uniform, 0, sizeof(SsaoSamplesUniform));*/
}
