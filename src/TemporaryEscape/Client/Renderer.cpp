#include "Renderer.hpp"
#include "../Utils/Exceptions.hpp"

#define CMP "Renderer"

using namespace Engine;

Renderer::Renderer(const Config& config, VulkanDevice& vulkan, Pipelines& pipelines) :
    config{config}, vulkan{vulkan}, pipelines{pipelines} {
    createFullScreenQuad();
    createSkyboxMesh();
}

Renderer::~Renderer() {
}

void Renderer::update(const Vector2i& viewport) {
    if (gBuffer.size != viewport && viewport.x != 0 && viewport.y != 0) {
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
        gBuffer.color = vulkan.createTexture(desc);

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
        gBuffer.pbrResult = vulkan.createTexture(desc);

        desc = VulkanTexture::Descriptor{};
        desc.size = viewport;
        desc.format = VulkanTexture::Format::VK_FORMAT_D32_SFLOAT;
        desc.levels = 1;
        desc.type = VulkanTexture::Type::VK_IMAGE_TYPE_2D;
        desc.usage = VulkanTexture::Usage::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                     VulkanTexture::Usage::VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        gBuffer.depth = vulkan.createTexture(desc);

        std::vector<VulkanFramebufferAttachment> attachments = {
            {gBuffer.color},     {gBuffer.pbrColor},  {gBuffer.pbrEmissive}, {gBuffer.pbrMetallicRoughnessAmbient},
            {gBuffer.pbrNormal}, {gBuffer.pbrResult}, {gBuffer.depth},
        };

        fbo = vulkan.createFramebuffer(viewport, attachments);

        vulkan.deviceWaitIdle();
        Log::d(CMP, "Renderer framebuffer created of size {}", gBuffer.size);
    }
}

void Renderer::begin() {
    vulkan.setViewport({0, 0}, gBuffer.size);
    vulkan.setScissor({0, 0}, gBuffer.size);
    vulkan.setViewportState();

    VulkanFramebufferAttachmentReference colorAttachment{};
    colorAttachment.clearValue.color = {0.0f, 0.0f, 0.0f, 0.0f};

    VulkanFramebufferAttachmentReference pbrColorAttachment{};
    pbrColorAttachment.clearValue.color = {0.0f, 0.0f, 0.0f, 0.0f};

    VulkanFramebufferAttachmentReference pbrEmissiveAttachment{};
    pbrEmissiveAttachment.clearValue.color = {0.0f, 0.0f, 0.0f, 0.0f};

    VulkanFramebufferAttachmentReference pbrMetallicRoughnessAmbientAttachment{};
    pbrMetallicRoughnessAmbientAttachment.clearValue.color = {0.0f, 0.0f, 0.0f, 0.0f};

    VulkanFramebufferAttachmentReference pbrNormalAttachment{};
    pbrNormalAttachment.clearValue.color = {0.0f, 0.0f, 0.0f, 0.0f};

    VulkanFramebufferAttachmentReference pbrResultAttachment{};
    pbrResultAttachment.clearValue.color = {0.0f, 0.0f, 0.0f, 0.0f};

    VulkanFramebufferAttachmentReference depthStencilAttachment{};
    depthStencilAttachment.clearValue.depthStencil.depth = 1.0f;
    depthStencilAttachment.clearValue.depthStencil.stencil = 0;

    vulkan.beginRenderPass(fbo, {
                                    colorAttachment,
                                    pbrColorAttachment,
                                    pbrEmissiveAttachment,
                                    pbrMetallicRoughnessAmbientAttachment,
                                    pbrNormalAttachment,
                                    pbrResultAttachment,
                                    depthStencilAttachment,
                                });
}

void Renderer::end() {
    vulkan.endRenderPass();
}

void Renderer::render(const Vector2i& viewport, View& view) {
    auto skybox = view.getSkybox();
    if (!skybox) {
        return;
    }

    try {
        view.renderPbr(viewport);
    } catch (...) {
        EXCEPTION_NESTED("Something went wrong during renderPbr()");
    }

    vulkan.nextSubpass();

    // Skybox
    vulkan.bindPipeline(pipelines.skybox);
    // vulkan.bindTexture(skybox->getTexture(), 0);
    renderSkyboxMesh();

    // Pbr
    vulkan.bindPipeline(pipelines.pbr);
    vulkan.bindTexture(gBuffer.pbrColor, 0);
    vulkan.bindTexture(gBuffer.pbrEmissive, 1);
    vulkan.bindTexture(gBuffer.pbrMetallicRoughnessAmbient, 2);
    vulkan.bindTexture(gBuffer.pbrNormal, 3);
    renderFullScreenQuad();

    try {
        view.renderFwd(viewport);
    } catch (...) {
        EXCEPTION_NESTED("Something went wrong during renderFwd()");
    }

    try {
        view.renderCanvas(viewport);
    } catch (...) {
        EXCEPTION_NESTED("Something went wrong during renderCanvas()");
    }
}

void Renderer::present() {
    vulkan.submitPresentQueue(gBuffer.color);
}

/*void Renderer::loadShaders() {
    if (!pipelines.pbr) {
        pipelines.pbr = vulkan.createPipeline({
            {config.shadersPath / "pass-pbr.frag", "", ShaderType::Fragment},
            {config.shadersPath / "pass-pbr.vert", "", ShaderType::Vertex},
        });
    } else {
        ready = true;
    }
}*/

void Renderer::createFullScreenQuad() {
    static const std::vector<Vector2> vertices = {
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
    });
}

void Renderer::renderFullScreenQuad() {
    vulkan.setDepthStencilState(false, false);
    VulkanBlendState blendState{};
    blendState.blendEnable = true;
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
    vulkan.drawIndexed(6, 1, 0, 0, 0);
}

void Renderer::createSkyboxMesh() {
    static const std::vector<uint32_t> indices = {
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
    });
}

void Renderer::renderSkyboxMesh() {
    vulkan.setDepthStencilState(false, true);
    VulkanBlendState blendState{};
    blendState.blendEnable = true;
    blendState.colorBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
    blendState.alphaBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
    blendState.srcColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE;
    blendState.dstColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendState.srcAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE;
    blendState.dstAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendState.colorWriteMask = VkColorComponentFlagBits::VK_COLOR_COMPONENT_FLAG_BITS_MAX_ENUM;
    vulkan.setBlendState({blendState});

    Matrix4 transform = glm::scale(Vector3{100.0f, 100.0f, 100.0f});
    vulkan.pushConstant(0, transform);
    vulkan.bindVertexBuffer(skyboxMesh.vbo, 0);
    vulkan.bindVertexInputFormat(skyboxMesh.vboFormat);
    vulkan.bindIndexBuffer(skyboxMesh.ibo, 0, VK_INDEX_TYPE_UINT32);
    vulkan.setInputAssembly(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    vulkan.drawIndexed(6 * 6, 1, 0, 0, 0);
}
