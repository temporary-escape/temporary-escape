#include "skybox_generator.hpp"

using namespace Engine;

static const auto skyboxDefaultColor = Color4{0.03f, 0.03f, 0.03f, 1.0f};

static const std::array<Matrix4, 6> captureViewMatrices = {
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
};

// Simple skybox box with two triangles per side.
static const std::vector<float> skyboxVertices = {
    // positions
    -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
    1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

    -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
    -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

    1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

    -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

    -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
    1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,
};

static const float PI = static_cast<float>(std::atan(1) * 4);

// The projection matrix that will be used to generate the skybox.
// This must be 90 degrees view (PI/2)
static const glm::mat4 captureProjectionMatrix = glm::perspective(PI / 2.0, 1.0, 0.1, 1000.0);

// For some reason the first side (prefilter) texture is always bad no matter what I do.
// Temporary workaround, render the first side one more time. This seems to work.
// TODO: Find why the first side never renders properly.
static const std::array<int, 7> sidesToRender = {0, 1, 2, 3, 4, 5, 0};

#define CMP "SkyboxGenerator"

struct StarVertex {
    Vector3 position;
    float brightness;
    Color4 color;
};

SkyboxGenerator::SkyboxGenerator(const Config& config, VulkanDevice& vulkan, SkyboxGenerator::Pipelines& pipelines) :
    config{config}, vulkan{vulkan}, pipelines{pipelines}, skybox{vulkan, skyboxDefaultColor} {

    prepareFbo();
    prepareNebulaMesh();
}

void SkyboxGenerator::updateSeed(const uint64_t seed) {
    Log::i(CMP, "Updating skybox generator seed: {}", seed);

    auto doWork = false;
    if ((this->seed && *this->seed != seed) || !this->seed) {
        Log::i(CMP, "Skybox generator seed is different, triggering work!");
        doWork = true;
    }

    if (doWork) {
        generate(seed);
    }
}

void SkyboxGenerator::generate(const uint64_t seed) {
    // We don't want to halt the rendering while the skybox is generating.
    // Therefore, we generate the skybox step by step.
    // TODO: Maybe put this on a separate graphics queue without splitting this into multiple steps.

    this->seed = seed;

    rng = std::mt19937_64{seed};

    prepareStars(starsSmall, 50000);
    prepareStars(starsLarge, 1000);
    prepareNebulaUbo();

    // Prepare texture
    jobs.emplace_back([this]() { prepareCubemap(); });

    // Render all sides
    for (auto side : sidesToRender) {
        // Render small stars
        jobs.emplace_back([this, side]() { renderStars(starsSmall, side, 0.10f, true); });
        jobs.emplace_back([this, side]() { renderStars(starsLarge, side, 0.20f, false); });

        // Render nebulas
        for (auto& ubo : nebula.ubos) {
            jobs.emplace_back([this, side, &ubo]() { renderNebula(ubo, side); });
        }

        // Copy to the target cubemap texture
        jobs.emplace_back([this, side]() { blit(main.texture, skybox.getTexture(), side, config.skyboxSize); });

        // Irradiance texture for the PBR
        jobs.emplace_back([this, side]() { renderIrradiance(side); });

        // Copy to the target irradiance texture
        jobs.emplace_back(
            [this, side]() { blit(irradiance.texture, skybox.getIrradiance(), side, config.skyboxIrradianceSize); });

        // Prefilter texture for the PBR
        for (auto level = 0; level < static_cast<int>(std::log2(config.skyboxPrefilterSize)); level++) {
            jobs.emplace_back([this, side, level]() { renderPrefilter(side, level); });

            // Copy to the target cubemap texture
            jobs.emplace_back([this, side, level]() {
                blitLevel(prefilter.texture, skybox.getPrefilter(), side, config.skyboxPrefilterSize, level);
            });
        }
    }
}

void SkyboxGenerator::render() {
    if (!jobs.empty()) {
        jobs.front()();
        jobs.pop_front();

        if (jobs.empty()) {
            Log::i(CMP, "Skybox generator finished!");
        }
    }
}

void SkyboxGenerator::prepareFbo() {
    auto desc = VulkanTexture::Descriptor{};
    desc.size = Vector2i{config.skyboxSize, config.skyboxSize};
    desc.format = VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
    desc.levels = 1;
    desc.layers = 1;
    desc.type = VulkanTexture::Type::VK_IMAGE_TYPE_2D;
    desc.viewType = VulkanTexture::ViewType::VK_IMAGE_VIEW_TYPE_2D;
    desc.usage = VulkanTexture::Usage::VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                 VulkanTexture::Usage::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                 VulkanTexture::Usage::VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                 VulkanTexture::Usage::VK_IMAGE_USAGE_SAMPLED_BIT |
                 VulkanTexture::Usage::VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    main.texture = vulkan.createTexture(desc);

    std::vector<VulkanFramebufferAttachment> attachments = {{main.texture}};
    main.fbo = vulkan.createFramebuffer(desc.size, attachments);

    desc.size = Vector2i{config.skyboxPrefilterSize, config.skyboxPrefilterSize};
    desc.format = VkFormat::VK_FORMAT_R16G16B16A16_SFLOAT;
    prefilter.texture = vulkan.createTexture(desc);

    attachments = {{prefilter.texture}};
    prefilter.fbo = vulkan.createFramebuffer(desc.size, attachments);

    desc.size = Vector2i{config.skyboxIrradianceSize, config.skyboxIrradianceSize};
    desc.format = VkFormat::VK_FORMAT_R16G16B16A16_SFLOAT;
    irradiance.texture = vulkan.createTexture(desc);

    attachments = {{irradiance.texture}};
    irradiance.fbo = vulkan.createFramebuffer(desc.size, attachments);
}

void SkyboxGenerator::prepareNebulaMesh() {
    nebula.vbo = vulkan.createBuffer(VulkanBuffer::Type::Vertex, VulkanBuffer::Usage::Dynamic,
                                     sizeof(float) * skyboxVertices.size());
    nebula.vbo.subData(skyboxVertices.data(), 0, sizeof(float) * skyboxVertices.size());

    nebula.vboFormat = vulkan.createVertexInputFormat({
        {
            0,
            {
                {0, 0, VulkanVertexInputFormat::Format::Vec3},
            },
        },
    });

    nebula.count = skyboxVertices.size();
}

void SkyboxGenerator::prepareCubemap() {
    const auto size = Vector2i{config.skyboxSize, config.skyboxSize};

    Log::d(CMP, "Preparing skybox cubemap of size: {}", size);

    auto desc = VulkanTexture::Descriptor{};
    desc.size = size;
    desc.format = VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
    desc.levels = static_cast<uint32_t>(std::log2(config.skyboxSize));
    desc.layers = 6;
    desc.type = VulkanTexture::Type::VK_IMAGE_TYPE_2D;
    desc.viewType = VulkanTexture::ViewType::VK_IMAGE_VIEW_TYPE_CUBE;
    desc.usage = VulkanTexture::Usage::VK_IMAGE_USAGE_SAMPLED_BIT |
                 VulkanTexture::Usage::VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                 VulkanTexture::Usage::VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    desc.addressModeU = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    desc.addressModeV = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    desc.addressModeW = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

    skybox.getTexture() = vulkan.createTexture(desc);

    desc.format = VkFormat::VK_FORMAT_R16G16B16A16_SFLOAT;
    desc.size = Vector2i{config.skyboxPrefilterSize, config.skyboxPrefilterSize};
    desc.levels = static_cast<uint32_t>(std::log2(config.skyboxPrefilterSize));
    skybox.getPrefilter() = vulkan.createTexture(desc);

    desc.format = VkFormat::VK_FORMAT_R16G16B16A16_SFLOAT;
    desc.size = Vector2i{config.skyboxIrradianceSize, config.skyboxIrradianceSize};
    desc.levels = static_cast<uint32_t>(std::log2(config.skyboxIrradianceSize));
    skybox.getIrradiance() = vulkan.createTexture(desc);
}

void SkyboxGenerator::prepareStars(Stars& stars, const size_t count) {
    Log::d(CMP, "Preparing skybox vertex buffer for: {} stars", count);

    std::uniform_real_distribution<float> distPosition(-1.0f, 1.0f);
    std::uniform_real_distribution<float> distColor(0.9f, 1.0f);
    std::uniform_real_distribution<float> distBrightness(0.5f, 1.0f);

    std::vector<StarVertex> vertices;
    vertices.resize(count);

    for (size_t i = 0; i < vertices.size(); i++) {
        auto& star = vertices[i];
        star.position = glm::normalize(Vector3{distPosition(rng), distPosition(rng), distPosition(rng)}) * 100.0f;
        star.color = Color4{distColor(rng), distColor(rng), distColor(rng), 1.0f};
        star.brightness = distBrightness(rng);
    }

    stars.count = count;

    stars.vbo = vulkan.createBuffer(VulkanBuffer::Type::Vertex, VulkanBuffer::Usage::Dynamic,
                                    sizeof(StarVertex) * vertices.size());
    stars.vbo.subData(vertices.data(), 0, sizeof(StarVertex) * vertices.size());

    stars.vboFormat = vulkan.createVertexInputFormat({
        {
            0,
            {
                {0, 0, VulkanVertexInputFormat::Format::Vec3},
                {1, 0, VulkanVertexInputFormat::Format::Float},
                {2, 0, VulkanVertexInputFormat::Format::Vec4},
            },
        },
    });
}

void SkyboxGenerator::renderStars(Stars& stars, const int side, const float particleSize, const bool clear) {
    Log::d(CMP, "Rendering skybox stars cube side index: {}", side);

    const auto viewport = Vector2i{config.skyboxSize, config.skyboxSize};

    VulkanFramebufferAttachmentReference colorAttachment{};
    if (clear) {
        colorAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
    } else {
        colorAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;
    }
    colorAttachment.clearValue.color = {0.0f, 0.0f, 0.0f, 1.0f};

    vulkan.beginRenderPass(main.fbo, {colorAttachment});

    vulkan.setViewport({0, 0}, viewport);
    vulkan.setScissor({0, 0}, viewport);
    vulkan.setViewportState();

    vulkan.bindPipeline(pipelines.stars);

    vulkan.pushConstant(0, captureViewMatrices[side]);
    vulkan.pushConstant(sizeof(Matrix4), captureProjectionMatrix);
    vulkan.pushConstant(sizeof(Matrix4) * 2, Vector2{particleSize, particleSize});

    vulkan.setDepthStencilState(false, false);
    VulkanBlendState blendState{};
    blendState.blendEnable = true;
    blendState.colorBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
    blendState.alphaBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
    blendState.srcColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_SRC_ALPHA;
    blendState.dstColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE;
    blendState.srcAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_SRC_ALPHA;
    blendState.dstAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE;
    blendState.colorWriteMask = VkColorComponentFlagBits::VK_COLOR_COMPONENT_FLAG_BITS_MAX_ENUM;
    vulkan.setBlendState({blendState});

    vulkan.bindVertexBuffer(stars.vbo, 0);
    vulkan.bindVertexInputFormat(stars.vboFormat);
    vulkan.setInputAssembly(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
    vulkan.draw(stars.count, 1, 0, 0);

    vulkan.endRenderPass();
}

void SkyboxGenerator::prepareNebulaUbo() {
    struct NebulaUniform {
        Color4 uColor;
        Vector4 uOffset;
        float uScale;
        float uIntensity;
        float uFalloff;
    };

    std::uniform_real_distribution<float> dist{0.0f, 1.0f};

    nebula.ubos.clear();

    while (true) {
        const auto scale = dist(rng) * 0.5f + 0.25f;
        const auto intensity = dist(rng) * 0.2f + 0.9f;
        const auto color = Color4{dist(rng), dist(rng), dist(rng), 1.0f};
        const auto falloff = dist(rng) * 3.0f + 3.0f;
        const auto offset =
            Vector3{dist(rng) * 2000.0f - 1000.0f, dist(rng) * 2000.0f - 1000.0f, dist(rng) * 2000.0f - 1000.0f};

        NebulaUniform uniform{};
        uniform.uColor = color;
        uniform.uOffset = Vector4{offset, 1.0f};
        uniform.uScale = scale;
        uniform.uIntensity = intensity;
        uniform.uFalloff = falloff;

        auto ubo = vulkan.createBuffer(VulkanBuffer::Type::Uniform, VulkanBuffer::Usage::Static, sizeof(NebulaUniform));
        ubo.subData(&uniform, 0, sizeof(NebulaUniform));

        nebula.ubos.push_back(std::move(ubo));

        if (dist(rng) < 0.5f) {
            break;
        }
    }
}

void SkyboxGenerator::renderNebula(VulkanBuffer& ubo, const int side) {
    Log::d(CMP, "Rendering skybox nebula for side index: {}", side);

    const auto viewport = Vector2i{config.skyboxSize, config.skyboxSize};

    VulkanFramebufferAttachmentReference colorAttachment{};
    colorAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;

    vulkan.beginRenderPass(main.fbo, {colorAttachment});

    vulkan.setViewport({0, 0}, viewport);
    vulkan.setScissor({0, 0}, viewport);
    vulkan.setViewportState();

    vulkan.bindPipeline(pipelines.nebula);

    vulkan.pushConstant(0, captureViewMatrices[side]);
    vulkan.pushConstant(sizeof(Matrix4), captureProjectionMatrix);
    vulkan.bindUniformBuffer(ubo, 0);

    vulkan.setDepthStencilState(false, false);
    VulkanBlendState blendState{};
    blendState.blendEnable = true;
    blendState.colorBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
    blendState.alphaBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
    blendState.srcColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_SRC_ALPHA;
    blendState.dstColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendState.srcAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE;
    blendState.dstAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ZERO;
    blendState.colorWriteMask = VkColorComponentFlagBits::VK_COLOR_COMPONENT_FLAG_BITS_MAX_ENUM;
    vulkan.setBlendState({blendState});

    vulkan.bindVertexBuffer(nebula.vbo, 0);
    vulkan.bindVertexInputFormat(nebula.vboFormat);
    vulkan.setInputAssembly(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    vulkan.draw(nebula.count, 1, 0, 0);

    vulkan.endRenderPass();
}

void SkyboxGenerator::renderIrradiance(const int side) {
    Log::d(CMP, "Rendering skybox irradiance texture for side index: {}", side);

    const auto viewport = Vector2i{config.skyboxIrradianceSize, config.skyboxIrradianceSize};

    VulkanFramebufferAttachmentReference colorAttachment{};
    colorAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;

    vulkan.beginRenderPass(irradiance.fbo, {colorAttachment});

    vulkan.setViewport({0, 0}, viewport);
    vulkan.setScissor({0, 0}, viewport);
    vulkan.setViewportState();

    vulkan.bindPipeline(pipelines.irradiance);

    vulkan.pushConstant(0, captureViewMatrices[side]);
    vulkan.pushConstant(sizeof(Matrix4), captureProjectionMatrix);
    vulkan.bindTexture(skybox.getTexture(), 0);

    vulkan.setDepthStencilState(false, false);
    VulkanBlendState blendState{};
    blendState.blendEnable = true;
    blendState.colorBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
    blendState.alphaBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
    blendState.srcColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_SRC_ALPHA;
    blendState.dstColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendState.srcAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE;
    blendState.dstAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ZERO;
    blendState.colorWriteMask = VkColorComponentFlagBits::VK_COLOR_COMPONENT_FLAG_BITS_MAX_ENUM;
    vulkan.setBlendState({blendState});

    vulkan.bindVertexBuffer(nebula.vbo, 0);
    vulkan.bindVertexInputFormat(nebula.vboFormat);
    vulkan.setInputAssembly(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    vulkan.draw(nebula.count, 1, 0, 0);

    vulkan.endRenderPass();
}

void SkyboxGenerator::renderPrefilter(const int side, const int level) {
    Log::d(CMP, "Rendering skybox prefilter texture for side index: {} level: {}", side, level);

    const auto viewport = Vector2i{config.skyboxPrefilterSize, config.skyboxPrefilterSize};

    VulkanFramebufferAttachmentReference colorAttachment{};
    colorAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;

    vulkan.beginRenderPass(prefilter.fbo, {colorAttachment});

    vulkan.setViewport({0, 0}, viewport);
    vulkan.setScissor({0, 0}, viewport);
    vulkan.setViewportState();

    vulkan.bindPipeline(pipelines.prefilter);

    const auto mipmaps = static_cast<int>(std::log2(viewport.x));
    const auto roughness = static_cast<float>(level) / static_cast<float>(mipmaps - 1);

    vulkan.pushConstant(0, captureViewMatrices[side]);
    vulkan.pushConstant(sizeof(Matrix4), captureProjectionMatrix);
    vulkan.pushConstant(sizeof(Matrix4) * 2, 0.0f);
    vulkan.bindTexture(skybox.getTexture(), 0);

    vulkan.setDepthStencilState(false, false);
    VulkanBlendState blendState{};
    blendState.blendEnable = true;
    blendState.colorBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
    blendState.alphaBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
    blendState.srcColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_SRC_ALPHA;
    blendState.dstColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendState.srcAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE;
    blendState.dstAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ZERO;
    blendState.colorWriteMask = VkColorComponentFlagBits::VK_COLOR_COMPONENT_FLAG_BITS_MAX_ENUM;
    vulkan.setBlendState({blendState});

    vulkan.bindVertexBuffer(nebula.vbo, 0);
    vulkan.bindVertexInputFormat(nebula.vboFormat);
    vulkan.setInputAssembly(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    vulkan.draw(nebula.count, 1, 0, 0);

    vulkan.endRenderPass();
}

void SkyboxGenerator::blit(VulkanTexture& source, VulkanTexture& target, const int side, const int width) {
    Log::d(CMP, "Copying skybox cubemap for side index: {}", side);

    const auto size = Vector2i{width, width};

    BlitImageInfo blitInfo{};
    blitInfo.src.mipLevel = 0;
    blitInfo.src.layerCount = 1;
    blitInfo.src.baseArrayLayer = 0;
    blitInfo.src.size = Vector3i{size.x, size.y, 1};

    blitInfo.dst.mipLevel = 0;
    blitInfo.dst.layerCount = 1;
    blitInfo.dst.baseArrayLayer = side;
    blitInfo.dst.size = Vector3i{size.x, size.y, 1};

    vulkan.blitImage(source, target, blitInfo, VkFilter::VK_FILTER_LINEAR);

    for (auto level = 1; level < static_cast<uint32_t>(std::log2(width)); level++) {
        blitInfo.src.mipLevel = 0;
        blitInfo.src.layerCount = 1;
        blitInfo.src.baseArrayLayer = 0;
        blitInfo.src.size = Vector3i{size.x, size.y, 1};

        blitInfo.dst.mipLevel = level;
        blitInfo.dst.layerCount = 1;
        blitInfo.dst.baseArrayLayer = side;
        blitInfo.dst.size = Vector3i{size.x >> level, size.y >> level, 1};

        vulkan.blitImage(source, target, blitInfo, VkFilter::VK_FILTER_LINEAR);
    }
}

/*void SkyboxGenerator::generateMipMaps(VulkanTexture& source, int side, int width) {
    const auto size = Vector2i{width, width};

    for (auto level = 1; level < static_cast<uint32_t>(std::log2(width)); level++) {
        BlitImageInfo blitInfo{};

        blitInfo.src.mipLevel = level - 1;
        blitInfo.src.layerCount = 1;
        blitInfo.src.baseArrayLayer = 0;
        blitInfo.src.size = Vector3i{size.x >> (level - 1), size.y >> (level - 1), 1};

        blitInfo.dst.mipLevel = level;
        blitInfo.dst.layerCount = 1;
        blitInfo.dst.baseArrayLayer = side;
        blitInfo.dst.size = Vector3i{size.x >> level, size.y >> level, 1};

        vulkan.blitImage(source, source, blitInfo, VkFilter::VK_FILTER_LINEAR);
    }
}*/

void SkyboxGenerator::blitLevel(VulkanTexture& source, VulkanTexture& target, const int side, const int width,
                                const int level) {
    Log::d(CMP, "Copying skybox cubemap for side index: {} level: {}", side, level);

    const auto size = Vector2i{width, width};

    BlitImageInfo blitInfo{};
    blitInfo.src.mipLevel = 0;
    blitInfo.src.layerCount = 1;
    blitInfo.src.baseArrayLayer = 0;
    blitInfo.src.size = Vector3i{size.x, size.y, 1};

    blitInfo.dst.mipLevel = level;
    blitInfo.dst.layerCount = 1;
    blitInfo.dst.baseArrayLayer = side;
    blitInfo.dst.size = Vector3i{size.x >> level, size.y >> level, 1};

    vulkan.blitImage(source, target, blitInfo, VkFilter::VK_FILTER_LINEAR);
}

bool SkyboxGenerator::isReady() const {
    return jobs.empty();
}
