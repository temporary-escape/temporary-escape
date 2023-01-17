#include "renderer.hpp"
#include "../utils/exceptions.hpp"

#define CMP "Renderer"

using namespace Engine;

struct FullScreenVertex {
    Vector2 pos;
};

Renderer::Renderer(const Config& config, VulkanRenderer& vulkan, Canvas& canvas, Nuklear& nuklear,
                   ShaderModules& shaderModules, VoxelShapeCache& voxelShapeCache, FontFamily& font) :
    config{config},
    vulkan{vulkan},
    canvas{canvas},
    nuklear{nuklear},
    voxelShapeCache{voxelShapeCache},
    font{font},
    lastViewportSize{config.windowWidth, config.windowHeight},
    bloomViewportSize{lastViewportSize / 2} {

    createGaussianKernel(15, 4.5);
    createFullScreenQuad();
    createSkyboxMesh();
    createSsaoNoise();
    createSsaoSamples();
    createRenderPasses();
    createShaders(shaderModules);

    renderBrdf();
}

Renderer::~Renderer() = default;

void Renderer::createSsaoNoise() {
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::random_device rd;
    std::default_random_engine generator{rd()};

    std::vector<Vector4> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++) {
        Vector4 noise{
            // rotate around z-axis (in tangent space)
            randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator) * 2.0 - 1.0,
            0.0f,
            0.0f,
        };
        ssaoNoise.push_back(noise);
    }

    VulkanTexture::CreateInfo textureInfo{};
    textureInfo.image.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    textureInfo.image.imageType = VK_IMAGE_TYPE_2D;
    textureInfo.image.extent = {4, 4, 1};
    textureInfo.image.mipLevels = 1;
    textureInfo.image.arrayLayers = 1;
    textureInfo.image.tiling = VK_IMAGE_TILING_OPTIMAL;
    textureInfo.image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    textureInfo.image.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    textureInfo.image.samples = VK_SAMPLE_COUNT_1_BIT;
    textureInfo.image.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    textureInfo.view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    textureInfo.view.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    textureInfo.view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    textureInfo.view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    textureInfo.view.subresourceRange.baseMipLevel = 0;
    textureInfo.view.subresourceRange.levelCount = 1;
    textureInfo.view.subresourceRange.baseArrayLayer = 0;
    textureInfo.view.subresourceRange.layerCount = 1;

    textureInfo.sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    textureInfo.sampler.magFilter = VK_FILTER_LINEAR;
    textureInfo.sampler.minFilter = VK_FILTER_LINEAR;
    textureInfo.sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.anisotropyEnable = VK_FALSE;
    textureInfo.sampler.maxAnisotropy = 1.0f;
    textureInfo.sampler.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    textureInfo.sampler.unnormalizedCoordinates = VK_FALSE;
    textureInfo.sampler.compareEnable = VK_FALSE;
    textureInfo.sampler.compareOp = VK_COMPARE_OP_ALWAYS;
    textureInfo.sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    ssaoSamples.noise = vulkan.createTexture(textureInfo);
    vulkan.transitionImageLayout(ssaoSamples.noise, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vulkan.copyDataToImage(ssaoSamples.noise, 0, {0, 0}, 0, {4, 4}, ssaoNoise.data());
    vulkan.transitionImageLayout(ssaoSamples.noise, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void Renderer::createSsaoSamples() {
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::random_device rd;
    std::default_random_engine generator{rd()};

    ShaderPassSSAO::SamplesUniform uniform{};

    for (unsigned int i = 0; i < sizeof(ShaderPassSSAO::SamplesUniform::weights) / sizeof(Vector4); ++i) {
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

    VulkanBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(ShaderPassSSAO::SamplesUniform);
    bufferInfo.usage =
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

    ssaoSamples.ubo = vulkan.createBuffer(bufferInfo);
    vulkan.copyDataToBuffer(ssaoSamples.ubo, &uniform, bufferInfo.size);
}

void Renderer::createGaussianKernel(const size_t size, double sigma) {
    const auto weights = gaussianKernel((size - 1) * 2 + 1, sigma);
    ShaderPassBloomBlur::GaussianWeightsUniform data;

    for (size_t i = 0; i < size; i++) {
        const auto w = static_cast<float>(weights[size - i - 1]);
        data.weight[i] = Vector4{w, w, w, 1.0f};
    }
    data.count = static_cast<int>(size);

    VulkanBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(ShaderPassBloomBlur::GaussianWeightsUniform);
    bufferInfo.usage =
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

    blurWeights.ubo = vulkan.createBuffer(bufferInfo);
    vulkan.copyDataToBuffer(blurWeights.ubo, &data, bufferInfo.size);
}

void Renderer::createFullScreenQuad() {
    static const std::vector<FullScreenVertex> vertices = {
        {{-1.0f, -1.0f}},
        {{1.0f, -1.0f}},
        {{1.0f, 1.0f}},
        {{-1.0f, 1.0f}},
    };

    static const std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0,
    };

    VulkanBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(FullScreenVertex) * vertices.size();
    bufferInfo.usage =
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

    meshes.fullScreenQuad.vbo = vulkan.createBuffer(bufferInfo);
    vulkan.copyDataToBuffer(meshes.fullScreenQuad.vbo, vertices.data(), sizeof(FullScreenVertex) * vertices.size());

    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(uint16_t) * indices.size();
    bufferInfo.usage =
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

    meshes.fullScreenQuad.ibo = vulkan.createBuffer(bufferInfo);
    vulkan.copyDataToBuffer(meshes.fullScreenQuad.ibo, indices.data(), sizeof(uint16_t) * indices.size());

    meshes.fullScreenQuad.indexType = VK_INDEX_TYPE_UINT16;
    meshes.fullScreenQuad.count = indices.size();
}

void Renderer::createSkyboxMesh() {
    static const std::vector<uint16_t> indices = {
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

    VulkanBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(Vector3) * vertices.size();
    bufferInfo.usage =
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

    meshes.skybox.vbo = vulkan.createBuffer(bufferInfo);
    vulkan.copyDataToBuffer(meshes.skybox.vbo, vertices.data(), sizeof(Vector3) * vertices.size());

    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(uint16_t) * indices.size();
    bufferInfo.usage =
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

    meshes.skybox.ibo = vulkan.createBuffer(bufferInfo);
    vulkan.copyDataToBuffer(meshes.skybox.ibo, indices.data(), sizeof(uint16_t) * indices.size());

    meshes.skybox.indexType = VK_INDEX_TYPE_UINT16;
    meshes.skybox.count = indices.size();
}

void Renderer::createShaders(ShaderModules& shaderModules) {
    VulkanSemaphore::CreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    compute.semaphore = vulkan.createSemaphore(semaphoreInfo);

    shaders.positionFeedback = ShaderPositionFeedback{
        config,
        vulkan,
        shaderModules,
    };
    shaders.brdf = ShaderBrdf{
        config,
        vulkan,
        shaderModules,
        renderPasses.brdf.renderPass,
    };
    shaders.componentGrid = ShaderComponentGrid{
        config,
        vulkan,
        shaderModules,
        renderPasses.pbr.renderPass,
    };
    shaders.passSsao = ShaderPassSSAO{
        config,
        vulkan,
        shaderModules,
        renderPasses.ssao.renderPass,
    };
    shaders.passSkybox = ShaderPassSkybox{
        config,
        vulkan,
        shaderModules,
        renderPasses.lighting.renderPass,
    };
    shaders.passDebug = ShaderPassDebug{
        config,
        vulkan,
        shaderModules,
        vulkan.getRenderPass(),
    };
    shaders.passPbr = ShaderPassPbr{
        config,
        vulkan,
        shaderModules,
        renderPasses.lighting.renderPass,
    };
    shaders.passFxaa = ShaderPassFXAA{
        config,
        vulkan,
        shaderModules,
        renderPasses.fxaa.renderPass,
    };
    shaders.passBloomExtract = ShaderPassBloomExtract{
        config,
        vulkan,
        shaderModules,
        renderPasses.bloomExtract.renderPass,
    };
    shaders.passBloomBlur[0] = ShaderPassBloomBlur{
        config,
        vulkan,
        shaderModules,
        renderPasses.bloomBlur[0].renderPass,
    };
    shaders.passBloomBlur[1] = ShaderPassBloomBlur{
        config,
        vulkan,
        shaderModules,
        renderPasses.bloomBlur[1].renderPass,
    };
    shaders.passBloomCombine = ShaderPassBloomCombine{
        config,
        vulkan,
        shaderModules,
        vulkan.getRenderPass(),
    };
    shaders.componentPointCloud = ShaderComponentPointCloud{
        config,
        vulkan,
        shaderModules,
        renderPasses.forward.renderPass,
    };
    shaders.componentDebug = ShaderComponentDebug{
        config,
        vulkan,
        shaderModules,
        renderPasses.forward.renderPass,
    };
    shaders.componentLines = ShaderComponentLines{
        config,
        vulkan,
        shaderModules,
        renderPasses.forward.renderPass,
    };
    shaders.componentPolyShape = ShaderComponentPolyShape{
        config,
        vulkan,
        shaderModules,
        renderPasses.forward.renderPass,
    };
    shaders.componentWorldText = ShaderComponentWorldText{
        config,
        vulkan,
        shaderModules,
        vulkan.getRenderPass(),
    };
}

void Renderer::createRenderPasses() {
    // vulkan.waitDeviceIdle();
    if (!renderPasses.brdf.renderPass) {
        createRenderPassBrdf();
    }
    createDepthTexture();
    createRenderPassPbr();
    createRenderPassSsao();
    createRenderPassLighting();
    createRenderPassForward();
    createRenderPassFxaa();
    createRenderPassBloomExtract();
    createRenderPassBloomBlur();
}

VkFormat Renderer::findDepthFormat() {
    return vulkan.findSupportedFormat(
        {
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT,
        },
        VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void Renderer::createDepthTexture() {
    textures.depthFormat = findDepthFormat();

    createAttachment(
        // Render pass
        textures.depth,
        // Size
        lastViewportSize,
        // Format
        textures.depthFormat,
        // Usage
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        // Aspect mask
        VK_IMAGE_ASPECT_DEPTH_BIT);
}

void Renderer::createRenderPassBrdf() {
    createAttachment(
        // Render pass
        textures.brdf,
        // Size
        {config.brdfSize, config.brdfSize},
        // Format
        VK_FORMAT_R16G16_SFLOAT,
        // Usage
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        // Aspect mask
        VK_IMAGE_ASPECT_COLOR_BIT);

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = VK_FORMAT_R16G16_SFLOAT;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VulkanRenderPass::CreateInfo renderPassInfo{};
    renderPassInfo.attachments = {colorAttachment};

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Use subpass dependencies for attachment layout transitions
    VkSubpassDependency dependency;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    renderPassInfo.subPasses = {subpass};
    renderPassInfo.dependencies = {dependency};

    renderPasses.brdf.renderPass = vulkan.createRenderPass(renderPassInfo);

    VkImageView attachments[] = {textures.brdf.getImageView()};

    VulkanFramebuffer::CreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPasses.brdf.renderPass.getHandle();
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = config.brdfSize;
    framebufferInfo.height = config.brdfSize;
    framebufferInfo.layers = 1;

    renderPasses.brdf.fbo = vulkan.createFramebuffer(framebufferInfo);
}

void Renderer::createRenderPassPbr() {
    // Albedo + AO
    createAttachment(
        // Render pass
        textures.pbrAlbedoAmbient,
        // Size
        lastViewportSize,
        // Format
        VK_FORMAT_R8G8B8A8_UNORM,
        // Usage
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        // Aspect mask
        VK_IMAGE_ASPECT_COLOR_BIT);

    // Emissive + Roughness
    createAttachment(
        // Render pass
        textures.pbrEmissiveRoughness,
        // Size
        lastViewportSize,
        // Format
        VK_FORMAT_R8G8B8A8_UNORM,
        // Usage
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        // Aspect mask
        VK_IMAGE_ASPECT_COLOR_BIT);

    // Normal + Metallic
    createAttachment(
        // Render pass
        textures.pbrNormalMetallic,
        // Size
        lastViewportSize,
        // Format
        VK_FORMAT_R8G8B8A8_UNORM,
        // Usage
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        // Aspect mask
        VK_IMAGE_ASPECT_COLOR_BIT);

    std::vector<VkAttachmentDescription> attachments{4};
    attachments[0].format = VK_FORMAT_R8G8B8A8_UNORM;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    attachments[1].format = VK_FORMAT_R8G8B8A8_UNORM;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    attachments[2].format = VK_FORMAT_R8G8B8A8_UNORM;
    attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[2].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    attachments[3].format = textures.depthFormat;
    attachments[3].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[3].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[3].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VulkanRenderPass::CreateInfo renderPassInfo{};
    renderPassInfo.attachments = attachments;

    std::array<VkAttachmentReference, 3> colorAttachmentRefs{};
    VkAttachmentReference depthAttachmentRef{};

    colorAttachmentRefs[0].attachment = 0;
    colorAttachmentRefs[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    colorAttachmentRefs[1].attachment = 1;
    colorAttachmentRefs[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    colorAttachmentRefs[2].attachment = 2;
    colorAttachmentRefs[2].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    depthAttachmentRef.attachment = 3;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Use subpass dependencies for attachment layout transitions
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
    subpass.pColorAttachments = colorAttachmentRefs.data();
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    renderPassInfo.subPasses = {subpass};
    renderPassInfo.dependencies = {dependency};

    renderPasses.pbr.renderPass = vulkan.createRenderPass(renderPassInfo);

    std::array<VkImageView, 4> attachmentViews = {
        textures.pbrAlbedoAmbient.getImageView(),
        textures.pbrEmissiveRoughness.getImageView(),
        textures.pbrNormalMetallic.getImageView(),
        textures.depth.getImageView(),
    };

    VulkanFramebuffer::CreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPasses.pbr.renderPass.getHandle();
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentViews.size());
    framebufferInfo.pAttachments = attachmentViews.data();
    framebufferInfo.width = lastViewportSize.x;
    framebufferInfo.height = lastViewportSize.y;
    framebufferInfo.layers = 1;

    renderPasses.pbr.fbo = vulkan.createFramebuffer(framebufferInfo);

    VulkanSemaphore::CreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    renderPasses.pbr.semaphore = vulkan.createSemaphore(semaphoreInfo);
}

void Renderer::createRenderPassSsao() {
    createAttachment(
        // Render pass
        textures.ssao,
        // Size
        lastViewportSize,
        // Format
        VK_FORMAT_R8_UNORM,
        // Usage
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        // Aspect mask
        VK_IMAGE_ASPECT_COLOR_BIT);

    std::vector<VkAttachmentDescription> attachments{1};
    attachments[0].format = VK_FORMAT_R8_UNORM;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VulkanRenderPass::CreateInfo renderPassInfo{};
    renderPassInfo.attachments = attachments;

    std::array<VkAttachmentReference, 1> colorAttachmentRefs{};

    colorAttachmentRefs[0].attachment = 0;
    colorAttachmentRefs[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Use subpass dependencies for attachment layout transitions
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
    subpass.pColorAttachments = colorAttachmentRefs.data();

    renderPassInfo.subPasses = {subpass};
    renderPassInfo.dependencies = {dependency};

    renderPasses.ssao.renderPass = vulkan.createRenderPass(renderPassInfo);

    std::array<VkImageView, 1> attachmentViews = {
        textures.ssao.getImageView(),
    };

    VulkanFramebuffer::CreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPasses.ssao.renderPass.getHandle();
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentViews.size());
    framebufferInfo.pAttachments = attachmentViews.data();
    framebufferInfo.width = lastViewportSize.x;
    framebufferInfo.height = lastViewportSize.y;
    framebufferInfo.layers = 1;

    renderPasses.ssao.fbo = vulkan.createFramebuffer(framebufferInfo);

    VulkanSemaphore::CreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    renderPasses.ssao.semaphore = vulkan.createSemaphore(semaphoreInfo);
}

void Renderer::createRenderPassLighting() {
    createAttachment(
        // Render pass
        textures.forward,
        // Size
        lastViewportSize,
        // Format
        VK_FORMAT_R16G16B16A16_SFLOAT,
        // Usage
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        // Aspect mask
        VK_IMAGE_ASPECT_COLOR_BIT);

    std::vector<VkAttachmentDescription> attachments{1};
    attachments[0].format = VK_FORMAT_R16G16B16A16_SFLOAT;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VulkanRenderPass::CreateInfo renderPassInfo{};
    renderPassInfo.attachments = attachments;

    std::array<VkAttachmentReference, 1> colorAttachmentRefs{};

    colorAttachmentRefs[0].attachment = 0;
    colorAttachmentRefs[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Use subpass dependencies for attachment layout transitions
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
    subpass.pColorAttachments = colorAttachmentRefs.data();

    renderPassInfo.subPasses = {subpass};
    renderPassInfo.dependencies = {dependency};

    renderPasses.lighting.renderPass = vulkan.createRenderPass(renderPassInfo);

    std::array<VkImageView, 1> attachmentViews = {
        textures.forward.getImageView(),
    };

    VulkanFramebuffer::CreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPasses.lighting.renderPass.getHandle();
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentViews.size());
    framebufferInfo.pAttachments = attachmentViews.data();
    framebufferInfo.width = lastViewportSize.x;
    framebufferInfo.height = lastViewportSize.y;
    framebufferInfo.layers = 1;

    renderPasses.lighting.fbo = vulkan.createFramebuffer(framebufferInfo);

    VulkanSemaphore::CreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    renderPasses.lighting.semaphore = vulkan.createSemaphore(semaphoreInfo);
}

void Renderer::createRenderPassForward() {
    std::vector<VkAttachmentDescription> attachments{2};
    attachments[0].format = VK_FORMAT_R16G16B16A16_SFLOAT;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    attachments[1].format = textures.depthFormat;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VulkanRenderPass::CreateInfo renderPassInfo{};
    renderPassInfo.attachments = attachments;

    std::array<VkAttachmentReference, 1> colorAttachmentRefs{};
    VkAttachmentReference depthAttachmentRef{};

    colorAttachmentRefs[0].attachment = 0;
    colorAttachmentRefs[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Use subpass dependencies for attachment layout transitions
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
    subpass.pColorAttachments = colorAttachmentRefs.data();
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    renderPassInfo.subPasses = {subpass};
    renderPassInfo.dependencies = {dependency};

    renderPasses.forward.renderPass = vulkan.createRenderPass(renderPassInfo);

    std::array<VkImageView, 2> attachmentViews = {
        textures.forward.getImageView(),
        textures.depth.getImageView(),
    };

    VulkanFramebuffer::CreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPasses.forward.renderPass.getHandle();
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentViews.size());
    framebufferInfo.pAttachments = attachmentViews.data();
    framebufferInfo.width = lastViewportSize.x;
    framebufferInfo.height = lastViewportSize.y;
    framebufferInfo.layers = 1;

    renderPasses.forward.fbo = vulkan.createFramebuffer(framebufferInfo);

    VulkanSemaphore::CreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    renderPasses.forward.semaphore = vulkan.createSemaphore(semaphoreInfo);
}

void Renderer::createRenderPassFxaa() {
    createAttachment(
        // Render pass
        textures.aux,
        // Size
        lastViewportSize,
        // Format
        VK_FORMAT_R16G16B16A16_SFLOAT,
        // Usage
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        // Aspect mask
        VK_IMAGE_ASPECT_COLOR_BIT);

    std::vector<VkAttachmentDescription> attachments{1};
    attachments[0].format = VK_FORMAT_R16G16B16A16_SFLOAT;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    VulkanRenderPass::CreateInfo renderPassInfo{};
    renderPassInfo.attachments = attachments;

    std::array<VkAttachmentReference, 1> colorAttachmentRefs{};

    colorAttachmentRefs[0].attachment = 0;
    colorAttachmentRefs[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Use subpass dependencies for attachment layout transitions
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
    subpass.pColorAttachments = colorAttachmentRefs.data();

    renderPassInfo.subPasses = {subpass};
    renderPassInfo.dependencies = {dependency};

    renderPasses.fxaa.renderPass = vulkan.createRenderPass(renderPassInfo);

    std::array<VkImageView, 1> attachmentViews = {
        textures.aux.getImageView(),
    };

    VulkanFramebuffer::CreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPasses.fxaa.renderPass.getHandle();
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentViews.size());
    framebufferInfo.pAttachments = attachmentViews.data();
    framebufferInfo.width = lastViewportSize.x;
    framebufferInfo.height = lastViewportSize.y;
    framebufferInfo.layers = 1;

    renderPasses.fxaa.fbo = vulkan.createFramebuffer(framebufferInfo);

    VulkanSemaphore::CreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    renderPasses.fxaa.semaphore = vulkan.createSemaphore(semaphoreInfo);
}

void Renderer::createRenderPassBloomExtract() {
    createAttachment(
        // Render pass
        textures.blurred[0],
        // Size
        bloomViewportSize,
        // Format
        VK_FORMAT_R16G16B16A16_SFLOAT,
        // Usage
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        // Aspect mask
        VK_IMAGE_ASPECT_COLOR_BIT);

    createAttachment(
        // Render pass
        textures.blurred[1],
        // Size
        bloomViewportSize,
        // Format
        VK_FORMAT_R16G16B16A16_SFLOAT,
        // Usage
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        // Aspect mask
        VK_IMAGE_ASPECT_COLOR_BIT);

    std::vector<VkAttachmentDescription> attachments{1};
    attachments[0].format = VK_FORMAT_R16G16B16A16_SFLOAT;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VulkanRenderPass::CreateInfo renderPassInfo{};
    renderPassInfo.attachments = attachments;

    std::array<VkAttachmentReference, 1> colorAttachmentRefs{};

    colorAttachmentRefs[0].attachment = 0;
    colorAttachmentRefs[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Use subpass dependencies for attachment layout transitions
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
    subpass.pColorAttachments = colorAttachmentRefs.data();

    renderPassInfo.subPasses = {subpass};
    renderPassInfo.dependencies = {dependency};

    renderPasses.bloomExtract.renderPass = vulkan.createRenderPass(renderPassInfo);

    std::array<VkImageView, 1> attachmentViews = {
        textures.blurred[0].getImageView(),
    };

    VulkanFramebuffer::CreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPasses.bloomExtract.renderPass.getHandle();
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentViews.size());
    framebufferInfo.pAttachments = attachmentViews.data();
    framebufferInfo.width = bloomViewportSize.x;
    framebufferInfo.height = bloomViewportSize.y;
    framebufferInfo.layers = 1;

    renderPasses.bloomExtract.fbo = vulkan.createFramebuffer(framebufferInfo);

    VulkanSemaphore::CreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    renderPasses.bloomExtract.semaphore = vulkan.createSemaphore(semaphoreInfo);
}

void Renderer::createRenderPassBloomBlur() {
    for (size_t i = 0; i < 2; i++) {
        std::vector<VkAttachmentDescription> attachments{1};
        attachments[0].format = VK_FORMAT_R16G16B16A16_SFLOAT;
        attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VulkanRenderPass::CreateInfo renderPassInfo{};
        renderPassInfo.attachments = attachments;

        std::array<VkAttachmentReference, 1> colorAttachmentRefs{};

        colorAttachmentRefs[0].attachment = 0;
        colorAttachmentRefs[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // Use subpass dependencies for attachment layout transitions
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
        subpass.pColorAttachments = colorAttachmentRefs.data();

        renderPassInfo.subPasses = {subpass};
        renderPassInfo.dependencies = {dependency};

        renderPasses.bloomBlur[i].renderPass = vulkan.createRenderPass(renderPassInfo);

        std::array<VkImageView, 1> attachmentViews = {
            textures.blurred[(i + 1) % 2].getImageView(),
        };

        VulkanFramebuffer::CreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPasses.bloomBlur[i].renderPass.getHandle();
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentViews.size());
        framebufferInfo.pAttachments = attachmentViews.data();
        framebufferInfo.width = bloomViewportSize.x;
        framebufferInfo.height = bloomViewportSize.y;
        framebufferInfo.layers = 1;

        renderPasses.bloomBlur[i].fbo = vulkan.createFramebuffer(framebufferInfo);

        VulkanSemaphore::CreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        renderPasses.bloomBlur[i].semaphore = vulkan.createSemaphore(semaphoreInfo);
    }
}

void Renderer::createAttachment(VulkanTexture& texture, const Vector2i& size, const VkFormat format,
                                const VkImageUsageFlags usage, const VkImageAspectFlags aspectMask) {

    VulkanTexture::CreateInfo textureInfo{};
    textureInfo.image.format = format;
    textureInfo.image.imageType = VK_IMAGE_TYPE_2D;
    textureInfo.image.extent = {static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y), 1};
    textureInfo.image.mipLevels = 1;
    textureInfo.image.arrayLayers = 1;
    textureInfo.image.tiling = VK_IMAGE_TILING_OPTIMAL;
    textureInfo.image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    textureInfo.image.usage = usage | VK_IMAGE_USAGE_SAMPLED_BIT;
    textureInfo.image.samples = VK_SAMPLE_COUNT_1_BIT;
    textureInfo.image.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    textureInfo.view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    textureInfo.view.format = format;
    textureInfo.view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    textureInfo.view.subresourceRange.aspectMask = aspectMask; // VK_IMAGE_ASPECT_COLOR_BIT;
    textureInfo.view.subresourceRange.baseMipLevel = 0;
    textureInfo.view.subresourceRange.levelCount = 1;
    textureInfo.view.subresourceRange.baseArrayLayer = 0;
    textureInfo.view.subresourceRange.layerCount = 1;

    textureInfo.sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    textureInfo.sampler.magFilter = VK_FILTER_LINEAR;
    textureInfo.sampler.minFilter = VK_FILTER_LINEAR;
    textureInfo.sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    textureInfo.sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    textureInfo.sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    textureInfo.sampler.anisotropyEnable = VK_FALSE;
    textureInfo.sampler.maxAnisotropy = 1.0f;
    textureInfo.sampler.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    textureInfo.sampler.unnormalizedCoordinates = VK_FALSE;
    textureInfo.sampler.compareEnable = VK_FALSE;
    textureInfo.sampler.compareOp = VK_COMPARE_OP_ALWAYS;
    textureInfo.sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    textureInfo.sampler.minLod = 0.0f;
    textureInfo.sampler.maxLod = 0.0f;

    texture = vulkan.createTexture(textureInfo);

    // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    // vulkan.transitionImageLayout(texture, VK_IMAGE_LAYOUT_UNDEFINED, layout);
}

void Renderer::renderMesh(VulkanCommandBuffer& vkb, RenderPassMesh& mesh) {
    std::array<VulkanVertexBufferBindRef, 1> vboBindings{};
    vboBindings[0] = {&mesh.vbo, 0};
    vkb.bindBuffers(vboBindings);
    vkb.bindIndexBuffer(mesh.ibo, 0, mesh.indexType);
    vkb.drawIndexed(mesh.count, 1, 0, 0, 0);
}

void Renderer::renderBrdf() {
    auto vkb = vulkan.createCommandBuffer();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkb.start(beginInfo);

    VulkanRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.framebuffer = &renderPasses.brdf.fbo;
    renderPassInfo.renderPass = &renderPasses.brdf.renderPass;
    renderPassInfo.offset = {0, 0};
    renderPassInfo.size = {config.brdfSize, config.brdfSize};

    VkClearValue clearColor = {{{1.0f, 0.0f, 1.0f, 1.0f}}};
    renderPassInfo.clearValues = {clearColor};

    vkb.beginRenderPass(renderPassInfo);

    vkb.bindPipeline(shaders.brdf.getPipeline());
    vkb.setViewport({0, 0}, renderPassInfo.size);
    vkb.setScissor({0, 0}, renderPassInfo.size);
    renderMesh(vkb, meshes.fullScreenQuad);

    vkb.endRenderPass();

    vkb.end();
    vulkan.submitCommandBuffer(vkb);
    vulkan.dispose(std::move(vkb));
    vulkan.waitQueueIdle();
}

void Renderer::render(const Vector2i& viewport, Scene& scene, const Skybox& skybox, const Options& options,
                      NuklearWindow& nuklearWindow) {

    if (viewport != lastViewportSize) {
        lastViewportSize = viewport;
        bloomViewportSize = lastViewportSize / 2;
        vulkan.waitDeviceIdle();
        createRenderPasses();
    }

    if (scene.getEntities().empty()) {
        EXCEPTION("Scene has no entities");
    }

    auto camera = scene.getPrimaryCamera();
    if (!camera) {
        EXCEPTION("Scene has no camera");
    }
    camera->recalculate(vulkan, viewport);

    // transitionDepthForWrite();
    renderPassCompute(viewport, scene, options);
    renderPassPbr(viewport, scene, options);
    renderPassSsao(viewport, scene, options);
    renderPassLighting(viewport, scene, skybox, options);
    transitionDepthForReadOnlyOptimal();
    renderPassForward(viewport, scene, skybox, options);
    renderPassFxaa(viewport);
    renderPassBloomExtract();
    renderPassBloomBlur();

    auto vkb = vulkan.createCommandBuffer();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkb.start(beginInfo);

    VulkanRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.framebuffer = &vulkan.getSwapChainFramebuffer();
    renderPassInfo.renderPass = &vulkan.getRenderPass();
    renderPassInfo.offset = {0, 0};
    renderPassInfo.size = viewport;

    VkClearValue clearColor = {{{0.1f, 0.1f, 0.1f, 1.0f}}};
    renderPassInfo.clearValues = {clearColor};

    vkb.beginRenderPass(renderPassInfo);
    vkb.setViewport({0, 0}, viewport);
    vkb.setScissor({0, 0}, viewport);

    renderPassBloomCombine(vkb, options);
    renderSceneForwardNonHDR(vkb, viewport, scene);
    renderSceneCanvas(vkb, viewport, scene);
    renderNuklear(vkb, viewport, nuklearWindow);

    vkb.endRenderPass();
    vkb.end();

    vulkan.submitCommandBuffer(vkb, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, renderPasses.bloomBlur[0].semaphore,
                               vulkan.getCurrentRenderFinishedSemaphore(), &vulkan.getCurrentInFlightFence());
    vulkan.dispose(std::move(vkb));
}

void Renderer::transitionDepthForReadOnlyOptimal() {
    vulkan.setDebugMessengerEnabled(false);

    auto vkb = vulkan.createCommandBuffer();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkb.start(beginInfo);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = textures.depth.getHandle();
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = textures.depth.getMipMaps();
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;

    VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    vkb.pipelineBarrier(sourceStage, destinationStage, barrier);

    vkb.end();

    vulkan.submitCommandBuffer(vkb);
    vulkan.dispose(std::move(vkb));

    vulkan.setDebugMessengerEnabled(true);
}

void Renderer::transitionForWrite(VulkanCommandBuffer& vkb, const size_t idx) {
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = textures.blurred[idx].getHandle();
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = textures.blurred[idx].getMipMaps();
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    vkb.pipelineBarrier(sourceStage, destinationStage, barrier);
}

void Renderer::renderPassCompute(const Vector2i& viewport, Scene& scene, const Renderer::Options& options) {
    auto camera = scene.getPrimaryCamera();

    auto vkb = vulkan.createComputeCommandBuffer();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkb.start(beginInfo);

    vkb.bindPipeline(shaders.positionFeedback.getPipeline());
    std::array<VulkanBufferBinding, 3> bufferBindings{};

    for (auto&& [entity, transform, component] : scene.getView<ComponentTransform, ComponentClickablePoints>().each()) {

        component.recalculate(vulkan);

        ShaderPositionFeedback::Uniforms constants{};
        constants.modelmatrix = transform.getAbsoluteTransform();
        constants.viewport = Vector2{viewport};
        constants.count = component.getCount();

        vkb.pushConstants(shaders.positionFeedback.getPipeline(), VK_SHADER_STAGE_COMPUTE_BIT, 0,
                          sizeof(ShaderPositionFeedback::Uniforms), &constants);

        bufferBindings[0] = {0, &camera->getUbo().getCurrentBuffer()};
        bufferBindings[1] = {1, &component.getBufferInput()};
        bufferBindings[2] = {2, &component.getBufferOutput()};

        vkb.bindDescriptors(shaders.positionFeedback.getPipeline(), shaders.positionFeedback.getDescriptorSetLayout(),
                            bufferBindings, {});

        const uint32_t workCount = (component.getCount() / 256) + 1;
        vkb.dispatch(workCount, 1, 1);
    }

    vkb.end();
    vulkan.submitCommandBuffer(vkb, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               vulkan.getCurrentImageAvailableSemaphore(), compute.semaphore, nullptr);
    vulkan.dispose(std::move(vkb));
}

void Renderer::renderPassPbr(const Vector2i& viewport, Scene& scene, const Renderer::Options& options) {
    auto vkb = vulkan.createCommandBuffer();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkb.start(beginInfo);

    VulkanRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.framebuffer = &renderPasses.pbr.fbo;
    renderPassInfo.renderPass = &renderPasses.pbr.renderPass;
    renderPassInfo.offset = {0, 0};
    renderPassInfo.size = viewport;

    renderPassInfo.clearValues.resize(4);
    // Albedo + AO
    renderPassInfo.clearValues[0].color = {{0.0f, 0.0f, 0.0f, 0.0f}};
    // Emissive + metallic
    renderPassInfo.clearValues[1].color = {{0.0f, 0.0f, 0.0f, 0.0f}};
    // Normal + roughness
    renderPassInfo.clearValues[2].color = {{0.0f, 0.0f, 0.0f, 0.0f}};
    // Depth + stencil
    renderPassInfo.clearValues[3].depthStencil.depth = 1.0f;
    renderPassInfo.clearValues[3].depthStencil.stencil = 0;

    vkb.beginRenderPass(renderPassInfo);
    vkb.setViewport({0, 0}, viewport);
    vkb.setScissor({0, 0}, viewport);

    renderSceneGrids(vkb, viewport, scene);

    vkb.endRenderPass();
    vkb.end();

    vulkan.submitCommandBuffer(vkb, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, compute.semaphore,
                               renderPasses.pbr.semaphore, nullptr);
    vulkan.dispose(std::move(vkb));
}

void Renderer::renderPassSsao(const Vector2i& viewport, Scene& scene, const Options& options) {
    auto camera = scene.getPrimaryCamera();

    auto vkb = vulkan.createCommandBuffer();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkb.start(beginInfo);

    VulkanRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.framebuffer = &renderPasses.ssao.fbo;
    renderPassInfo.renderPass = &renderPasses.ssao.renderPass;
    renderPassInfo.offset = {0, 0};
    renderPassInfo.size = viewport;

    renderPassInfo.clearValues.resize(1);
    renderPassInfo.clearValues[0].color = {{1.0f, 0.0f, 0.0f, 0.0f}};

    vkb.beginRenderPass(renderPassInfo);
    vkb.setViewport({0, 0}, viewport);
    vkb.setScissor({0, 0}, viewport);

    vkb.bindPipeline(shaders.passSsao.getPipeline());

    std::array<VulkanBufferBinding, 2> bufferBindings{};
    bufferBindings[0] = {0, &camera->getUbo().getCurrentBuffer()};
    bufferBindings[1] = {1, &ssaoSamples.ubo};

    std::array<VulkanTextureBinding, 3> textureBindings{};
    textureBindings[0] = {2, &textures.depth};
    textureBindings[1] = {3, &textures.pbrNormalMetallic};
    textureBindings[2] = {4, &ssaoSamples.noise};

    vkb.bindDescriptors(shaders.passSsao.getPipeline(), shaders.passSsao.getDescriptorSetLayout(), bufferBindings,
                        textureBindings);

    ShaderPassSSAO::Uniforms constants{};
    constants.scale = Vector2{viewport} / 4.0f;
    vkb.pushConstants(shaders.passSsao.getPipeline(),
                      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT, 0,
                      sizeof(ShaderPassSSAO::Uniforms), &constants);

    renderMesh(vkb, meshes.fullScreenQuad);

    vkb.endRenderPass();
    vkb.end();

    vulkan.submitCommandBuffer(vkb, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, renderPasses.pbr.semaphore,
                               renderPasses.ssao.semaphore, nullptr);
    vulkan.dispose(std::move(vkb));
}

void Renderer::renderPassLighting(const Vector2i& viewport, Scene& scene, const Skybox& skybox,
                                  const Options& options) {
    auto camera = scene.getPrimaryCamera();

    auto vkb = vulkan.createCommandBuffer();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkb.start(beginInfo);

    VulkanRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.framebuffer = &renderPasses.lighting.fbo;
    renderPassInfo.renderPass = &renderPasses.lighting.renderPass;
    renderPassInfo.offset = {0, 0};
    renderPassInfo.size = viewport;
    renderPassInfo.clearValues.resize(2);

    vkb.beginRenderPass(renderPassInfo);
    vkb.setViewport({0, 0}, viewport);
    vkb.setScissor({0, 0}, viewport);

    renderSceneSkybox(vkb, viewport, scene, skybox);
    renderLightingPbr(vkb, viewport, scene, skybox);

    vkb.endRenderPass();

    vkb.end();

    vulkan.submitCommandBuffer(vkb, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, renderPasses.ssao.semaphore,
                               renderPasses.lighting.semaphore, nullptr);
    vulkan.dispose(std::move(vkb));
}

void Renderer::renderPassForward(const Vector2i& viewport, Scene& scene, const Skybox& skybox, const Options& options) {
    auto camera = scene.getPrimaryCamera();

    auto vkb = vulkan.createCommandBuffer();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkb.start(beginInfo);

    VulkanRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.framebuffer = &renderPasses.forward.fbo;
    renderPassInfo.renderPass = &renderPasses.forward.renderPass;
    renderPassInfo.offset = {0, 0};
    renderPassInfo.size = viewport;
    renderPassInfo.clearValues.resize(1);
    renderPassInfo.clearValues[0].color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    vkb.beginRenderPass(renderPassInfo);
    vkb.setViewport({0, 0}, viewport);
    vkb.setScissor({0, 0}, viewport);

    renderSceneForward(vkb, viewport, scene);

    vkb.endRenderPass();

    vkb.end();

    vulkan.submitCommandBuffer(vkb, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, renderPasses.lighting.semaphore,
                               renderPasses.forward.semaphore, nullptr);
    vulkan.dispose(std::move(vkb));
}

void Renderer::renderPassFxaa(const Vector2i& viewport) {
    auto vkb = vulkan.createCommandBuffer();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkb.start(beginInfo);

    VulkanRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.framebuffer = &renderPasses.fxaa.fbo;
    renderPassInfo.renderPass = &renderPasses.fxaa.renderPass;
    renderPassInfo.offset = {0, 0};
    renderPassInfo.size = viewport;

    renderPassInfo.clearValues.resize(1);
    renderPassInfo.clearValues[0].color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    vkb.beginRenderPass(renderPassInfo);
    vkb.setViewport({0, 0}, viewport);
    vkb.setScissor({0, 0}, viewport);

    vkb.bindPipeline(shaders.passFxaa.getPipeline());

    std::array<VulkanTextureBinding, 1> textureBindings{};
    textureBindings[0] = {0, &textures.forward};

    vkb.bindDescriptors(shaders.passFxaa.getPipeline(), shaders.passFxaa.getDescriptorSetLayout(), {}, textureBindings);

    ShaderPassFXAA::Uniforms constants{};
    constants.textureSize = Vector2{viewport};
    vkb.pushConstants(shaders.passFxaa.getPipeline(),
                      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT, 0,
                      sizeof(ShaderPassFXAA::Uniforms), &constants);

    renderMesh(vkb, meshes.fullScreenQuad);

    vkb.endRenderPass();

    std::array<VkImageBlit, 1> blit{};
    blit[0].srcOffsets[0] = {0, 0, 0};
    blit[0].srcOffsets[1] = {
        static_cast<int32_t>(textures.aux.getExtent().width),
        static_cast<int32_t>(textures.aux.getExtent().height),
        1,
    };
    blit[0].srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit[0].srcSubresource.mipLevel = 0;
    blit[0].srcSubresource.baseArrayLayer = 0;
    blit[0].srcSubresource.layerCount = 1;
    blit[0].dstOffsets[0] = {0, 0, 0};
    blit[0].dstOffsets[1] = {
        static_cast<int32_t>(textures.blurred[1].getExtent().width),
        static_cast<int32_t>(textures.blurred[1].getExtent().height),
        1,
    };
    blit[0].dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit[0].dstSubresource.mipLevel = 0;
    blit[0].dstSubresource.baseArrayLayer = 0;
    blit[0].dstSubresource.layerCount = 1;

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = textures.blurred[1].getHandle();
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = textures.blurred[1].getMipMaps();
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkb.pipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, barrier);

    vkb.blitImage(textures.aux, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, textures.blurred[1],
                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, blit, VK_FILTER_LINEAR);

    barrier = VkImageMemoryBarrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = textures.blurred[1].getHandle();
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = textures.blurred[1].getMipMaps();
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkb.pipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, barrier);

    barrier = VkImageMemoryBarrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = textures.aux.getHandle();
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = textures.aux.getMipMaps();
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkb.pipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, barrier);

    vkb.end();

    vulkan.submitCommandBuffer(vkb, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, renderPasses.forward.semaphore,
                               renderPasses.fxaa.semaphore, nullptr);
    vulkan.dispose(std::move(vkb));
}

void Renderer::renderPassBloomExtract() {
    auto vkb = vulkan.createCommandBuffer();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkb.start(beginInfo);

    VulkanRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.framebuffer = &renderPasses.bloomExtract.fbo;
    renderPassInfo.renderPass = &renderPasses.bloomExtract.renderPass;
    renderPassInfo.offset = {0, 0};
    renderPassInfo.size = bloomViewportSize;
    renderPassInfo.clearValues.resize(1);
    renderPassInfo.clearValues[0].color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    vkb.beginRenderPass(renderPassInfo);
    vkb.setViewport({0, 0}, bloomViewportSize);
    vkb.setScissor({0, 0}, bloomViewportSize);

    vkb.bindPipeline(shaders.passBloomExtract.getPipeline());

    std::array<VulkanTextureBinding, 1> textureBindings{};
    textureBindings[0] = {0, &textures.blurred[1]};

    vkb.bindDescriptors(shaders.passBloomExtract.getPipeline(), shaders.passBloomExtract.getDescriptorSetLayout(), {},
                        textureBindings);

    ShaderPassBloomExtract::Uniforms constants{};
    constants.threshold = 0.5f;
    vkb.pushConstants(shaders.passBloomExtract.getPipeline(),
                      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT, 0,
                      sizeof(ShaderPassFXAA::Uniforms), &constants);

    renderMesh(vkb, meshes.fullScreenQuad);

    vkb.endRenderPass();
    vkb.end();

    vulkan.submitCommandBuffer(vkb, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, renderPasses.fxaa.semaphore,
                               renderPasses.bloomExtract.semaphore, nullptr);
    vulkan.dispose(std::move(vkb));
}

void Renderer::renderPassBloomBlur() {
    auto vkb = vulkan.createCommandBuffer();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkb.start(beginInfo);

    transitionForWrite(vkb, 1);
    renderPassBloomBlur(vkb, 0, false);
    transitionForWrite(vkb, 0);
    renderPassBloomBlur(vkb, 1, false);
    transitionForWrite(vkb, 1);
    renderPassBloomBlur(vkb, 0, false);

    transitionForWrite(vkb, 0);
    renderPassBloomBlur(vkb, 1, true);
    transitionForWrite(vkb, 1);
    renderPassBloomBlur(vkb, 0, true);
    transitionForWrite(vkb, 0);
    renderPassBloomBlur(vkb, 1, true);

    vkb.end();

    vulkan.submitCommandBuffer(vkb, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, renderPasses.bloomExtract.semaphore,
                               renderPasses.bloomBlur[0].semaphore, nullptr);
    vulkan.dispose(std::move(vkb));
}

void Renderer::renderPassBloomBlur(VulkanCommandBuffer& vkb, const size_t idx, const bool vertical) {
    VulkanRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.framebuffer = &renderPasses.bloomBlur[idx].fbo;
    renderPassInfo.renderPass = &renderPasses.bloomBlur[idx].renderPass;
    renderPassInfo.offset = {0, 0};
    renderPassInfo.size = bloomViewportSize;

    renderPassInfo.clearValues.resize(1);
    renderPassInfo.clearValues[0].color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    vkb.beginRenderPass(renderPassInfo);
    vkb.setViewport({0, 0}, bloomViewportSize);
    vkb.setScissor({0, 0}, bloomViewportSize);

    vkb.bindPipeline(shaders.passBloomBlur[idx].getPipeline());

    std::array<VulkanTextureBinding, 1> textureBindings{};
    textureBindings[0] = {1, &textures.blurred[idx]};

    std::array<VulkanBufferBinding, 1> bufferBindings{};
    bufferBindings[0] = {0, &blurWeights.ubo};

    vkb.bindDescriptors(shaders.passBloomBlur[idx].getPipeline(), shaders.passBloomBlur[idx].getDescriptorSetLayout(),
                        bufferBindings, textureBindings);

    ShaderPassBloomBlur::Uniforms constants{};
    constants.horizontal = !vertical;
    vkb.pushConstants(shaders.passBloomBlur[idx].getPipeline(),
                      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT, 0,
                      sizeof(ShaderPassFXAA::Uniforms), &constants);

    renderMesh(vkb, meshes.fullScreenQuad);

    vkb.endRenderPass();
}

void Renderer::renderPassBloomCombine(VulkanCommandBuffer& vkb, const Options& options) {
    vkb.bindPipeline(shaders.passBloomCombine.getPipeline());

    std::array<VulkanTextureBinding, 2> textureBindings{};
    textureBindings[0] = {0, &textures.aux};
    textureBindings[1] = {1, &textures.blurred[0]};

    vkb.bindDescriptors(shaders.passBloomCombine.getPipeline(), shaders.passBloomCombine.getDescriptorSetLayout(), {},
                        textureBindings);

    ShaderPassBloomCombine::Uniforms constants{};
    constants.exposure = config.vulkan.exposure;
    constants.gamma = config.vulkan.gamma;
    constants.bloomStrength = config.vulkan.bloomStrength;
    constants.bloomPower = config.vulkan.bloomPower;
    constants.contrast = config.vulkan.contrast;

    if (!options.bloomEnabled) {
        constants.bloomStrength = 0.0f;
    }

    vkb.pushConstants(shaders.passBloomCombine.getPipeline(),
                      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT, 0,
                      sizeof(ShaderPassBloomCombine::Uniforms), &constants);

    renderMesh(vkb, meshes.fullScreenQuad);
}

static void validateMaterial(const Material& material) {
    if (!material.ubo) {
        EXCEPTION("Primitive has no material uniform buffer allocated");
    }

    if (!material.baseColorTexture || !material.baseColorTexture->getVulkanTexture()) {
        EXCEPTION("Primitive has no base color texture");
    }

    if (!material.emissiveTexture || !material.emissiveTexture->getVulkanTexture()) {
        EXCEPTION("Primitive has no emissive texture");
    }

    if (!material.normalTexture || !material.normalTexture->getVulkanTexture()) {
        EXCEPTION("Primitive has no normal texture");
    }

    if (!material.ambientOcclusionTexture || !material.ambientOcclusionTexture->getVulkanTexture()) {
        EXCEPTION("Primitive has no ambient occlusion texture");
    }

    if (!material.metallicRoughnessTexture || !material.metallicRoughnessTexture->getVulkanTexture()) {
        EXCEPTION("Primitive has no metallic roughness texture");
    }
}

void Renderer::renderSceneGrids(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene) {
    auto systemGrids = scene.getView<ComponentTransform, ComponentGrid>();
    auto camera = scene.getPrimaryCamera();

    std::array<VulkanBufferBinding, 2> bufferBindings{};
    std::array<VulkanTextureBinding, 5> textureBindings{};
    std::array<VulkanVertexBufferBindRef, 1> vboBindings{};

    bufferBindings[0] = {0, &camera->getUbo().getCurrentBuffer()};

    vkb.bindPipeline(shaders.componentGrid.getPipeline());

    for (auto&& [entity, transform, grid] : systemGrids.each()) {
        grid.recalculate(vulkan, voxelShapeCache);

        ShaderComponentGrid::Uniforms uniforms{};
        uniforms.modelMatrix = transform.getAbsoluteTransform();
        uniforms.normalMatrix = glm::transpose(glm::inverse(glm::mat3x3(uniforms.modelMatrix)));

        vkb.pushConstants(shaders.componentGrid.getPipeline(),
                          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT, 0,
                          sizeof(ShaderComponentGrid::Uniforms), &uniforms);

        for (auto& primitive : grid.getPrimitives()) {
            if (!primitive.material) {
                EXCEPTION("Primitive has no material");
            }

            validateMaterial(*primitive.material);

            vboBindings[0] = {&primitive.vbo, 0};

            vkb.bindBuffers(vboBindings);
            vkb.bindIndexBuffer(primitive.ibo, 0, primitive.indexType);

            textureBindings[0] = {2, &primitive.material->baseColorTexture->getVulkanTexture()};
            textureBindings[1] = {3, &primitive.material->emissiveTexture->getVulkanTexture()};
            textureBindings[2] = {4, &primitive.material->normalTexture->getVulkanTexture()};
            textureBindings[3] = {5, &primitive.material->ambientOcclusionTexture->getVulkanTexture()};
            textureBindings[4] = {6, &primitive.material->metallicRoughnessTexture->getVulkanTexture()};

            bufferBindings[1] = {1, &primitive.material->ubo};

            vkb.bindDescriptors(shaders.componentGrid.getPipeline(), shaders.componentGrid.getDescriptorSetLayout(),
                                bufferBindings, textureBindings);

            vkb.drawIndexed(primitive.count, 1, 0, 0, 0);
        }
    }
}

void Renderer::renderSceneSkybox(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene,
                                 const Skybox& skybox) {
    auto camera = scene.getPrimaryCamera();

    vkb.bindPipeline(shaders.passSkybox.getPipeline());

    std::array<VulkanBufferBinding, 1> bufferBindings{};
    bufferBindings[0] = {0, &camera->getUboZeroPos().getCurrentBuffer()};

    std::array<VulkanTextureBinding, 1> textureBindings{};
    textureBindings[0] = {1, &skybox.getTexture()};

    vkb.bindDescriptors(shaders.passSkybox.getPipeline(), shaders.passSkybox.getDescriptorSetLayout(), bufferBindings,
                        textureBindings);

    ShaderPassSkybox::Uniforms constants{};
    constants.modelMatrix = glm::scale(Matrix4{1.0f}, Vector3{1000.0f});
    vkb.pushConstants(shaders.passSkybox.getPipeline(),
                      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT, 0,
                      sizeof(ShaderPassSkybox::Uniforms), &constants);

    renderMesh(vkb, meshes.skybox);
}

void Renderer::renderSceneForward(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene) {
    auto& camera = *scene.getPrimaryCamera();
    currentForwardShader = nullptr;

    std::vector<ForwardRenderJob> jobs;
    collectForRender<ComponentDebug>(vkb, viewport, scene, jobs);
    collectForRender<ComponentIconPointCloud>(vkb, viewport, scene, jobs);
    collectForRender<ComponentPointCloud>(vkb, viewport, scene, jobs);
    collectForRender<ComponentLines>(vkb, viewport, scene, jobs);
    collectForRender<ComponentPolyShape>(vkb, viewport, scene, jobs);

    std::sort(jobs.begin(), jobs.end(), [](auto& a, auto& b) { return a.order > b.order; });

    for (auto& job : jobs) {
        job.fn();
    }
}

void Renderer::renderSceneForwardNonHDR(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene) {
    auto& camera = *scene.getPrimaryCamera();
    currentForwardShader = nullptr;

    std::vector<ForwardRenderJob> jobs;
    collectForRender<ComponentWorldText>(vkb, viewport, scene, jobs);

    std::sort(jobs.begin(), jobs.end(), [](auto& a, auto& b) { return a.order > b.order; });

    for (auto& job : jobs) {
        job.fn();
    }
}

void Renderer::renderSceneForward(VulkanCommandBuffer& vkb, const ComponentCamera& camera,
                                  ComponentTransform& transform, ComponentDebug& component) {
    component.recalculate(vulkan);

    const auto& mesh = component.getMesh();

    if (mesh.count == 0) {
        return;
    }

    if (currentForwardShader != &shaders.componentDebug) {
        currentForwardShader = &shaders.componentDebug;
        vkb.bindPipeline(shaders.componentDebug.getPipeline());
    }

    std::array<VulkanBufferBinding, 1> bufferBindings{};
    bufferBindings[0] = {0, &camera.getUbo().getCurrentBuffer()};

    vkb.bindDescriptors(shaders.componentDebug.getPipeline(), shaders.componentDebug.getDescriptorSetLayout(),
                        bufferBindings, {});

    ShaderComponentDebug::Uniforms constants{};
    constants.modelMatrix = transform.getAbsoluteTransform();
    vkb.pushConstants(shaders.componentDebug.getPipeline(),
                      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT, 0,
                      sizeof(ShaderComponentDebug::Uniforms), &constants);

    renderMesh(vkb, mesh);
}

void Renderer::renderSceneForward(VulkanCommandBuffer& vkb, const ComponentCamera& camera,
                                  ComponentTransform& transform, ComponentIconPointCloud& component) {
    component.recalculate(vulkan);

    if (currentForwardShader != &shaders.componentPointCloud) {
        currentForwardShader = &shaders.componentPointCloud;
        vkb.bindPipeline(shaders.componentPointCloud.getPipeline());
    }

    for (const auto& [image, mesh] : component.getMesges()) {
        if (mesh.count == 0) {
            continue;
        }

        std::array<VulkanBufferBinding, 1> bufferBindings{};
        bufferBindings[0] = {0, &camera.getUbo().getCurrentBuffer()};

        std::array<VulkanTextureBinding, 1> textureBindings{};
        textureBindings[0] = {1, image->getAllocation().texture};

        vkb.bindDescriptors(shaders.componentPointCloud.getPipeline(),
                            shaders.componentPointCloud.getDescriptorSetLayout(), bufferBindings, textureBindings);

        ShaderComponentDebug::Uniforms constants{};
        constants.modelMatrix = transform.getAbsoluteTransform();
        vkb.pushConstants(shaders.componentPointCloud.getPipeline(),
                          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT, 0,
                          sizeof(ShaderComponentDebug::Uniforms), &constants);

        renderMesh(vkb, mesh);
    }
}

void Renderer::renderSceneForward(VulkanCommandBuffer& vkb, const ComponentCamera& camera,
                                  ComponentTransform& transform, ComponentPointCloud& component) {
    component.recalculate(vulkan);

    const auto& mesh = component.getMesh();
    if (mesh.count == 0) {
        return;
    }

    if (currentForwardShader != &shaders.componentPointCloud) {
        currentForwardShader = &shaders.componentPointCloud;
        vkb.bindPipeline(shaders.componentPointCloud.getPipeline());
    }

    std::array<VulkanBufferBinding, 1> bufferBindings{};
    bufferBindings[0] = {0, &camera.getUbo().getCurrentBuffer()};

    std::array<VulkanTextureBinding, 1> textureBindings{};
    textureBindings[0] = {1, &component.getTexture()->getVulkanTexture()};

    vkb.bindDescriptors(shaders.componentPointCloud.getPipeline(), shaders.componentPointCloud.getDescriptorSetLayout(),
                        bufferBindings, textureBindings);

    ShaderComponentDebug::Uniforms constants{};
    constants.modelMatrix = transform.getAbsoluteTransform();
    vkb.pushConstants(shaders.componentPointCloud.getPipeline(),
                      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT, 0,
                      sizeof(ShaderComponentDebug::Uniforms), &constants);

    renderMesh(vkb, mesh);
}

void Renderer::renderSceneForward(VulkanCommandBuffer& vkb, const ComponentCamera& camera,
                                  ComponentTransform& transform, ComponentLines& component) {
    component.recalculate(vulkan);

    const auto& mesh = component.getMesh();
    if (mesh.count == 0) {
        return;
    }

    if (currentForwardShader != &shaders.componentLines) {
        currentForwardShader = &shaders.componentLines;
        vkb.bindPipeline(shaders.componentLines.getPipeline());
    }

    std::array<VulkanBufferBinding, 1> bufferBindings{};
    bufferBindings[0] = {0, &camera.getUbo().getCurrentBuffer()};

    vkb.bindDescriptors(shaders.componentLines.getPipeline(), shaders.componentLines.getDescriptorSetLayout(),
                        bufferBindings, {});

    ShaderComponentLines::Uniforms constants{};
    constants.modelMatrix = transform.getAbsoluteTransform();
    constants.color = component.getColor();
    vkb.pushConstants(shaders.componentLines.getPipeline(),
                      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT, 0,
                      sizeof(ShaderComponentLines::Uniforms), &constants);

    renderMesh(vkb, mesh);
}

void Renderer::renderSceneForward(VulkanCommandBuffer& vkb, const ComponentCamera& camera,
                                  ComponentTransform& transform, ComponentPolyShape& component) {
    component.recalculate(vulkan);

    const auto& mesh = component.getMesh();
    if (mesh.count == 0) {
        return;
    }

    if (currentForwardShader != &shaders.componentPolyShape) {
        currentForwardShader = &shaders.componentPolyShape;
        vkb.bindPipeline(shaders.componentPolyShape.getPipeline());
    }

    std::array<VulkanBufferBinding, 1> bufferBindings{};
    bufferBindings[0] = {0, &camera.getUbo().getCurrentBuffer()};

    vkb.bindDescriptors(shaders.componentPolyShape.getPipeline(), shaders.componentPolyShape.getDescriptorSetLayout(),
                        bufferBindings, {});

    ShaderComponentPolyShape::Uniforms constants{};
    constants.modelMatrix = transform.getAbsoluteTransform();
    vkb.pushConstants(shaders.componentPolyShape.getPipeline(),
                      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT, 0,
                      sizeof(ShaderComponentPolyShape::Uniforms), &constants);

    renderMesh(vkb, mesh);
}

void Renderer::renderSceneForward(VulkanCommandBuffer& vkb, const ComponentCamera& camera,
                                  ComponentTransform& transform, ComponentWorldText& component) {
    component.recalculate(vulkan);

    const auto& mesh = component.getMesh();
    if (mesh.count == 0) {
        return;
    }

    if (currentForwardShader != &shaders.componentWorldText) {
        currentForwardShader = &shaders.componentWorldText;
        vkb.bindPipeline(shaders.componentWorldText.getPipeline());
    }

    std::array<VulkanBufferBinding, 1> bufferBindings{};
    bufferBindings[0] = {0, &camera.getUbo().getCurrentBuffer()};

    std::array<VulkanTextureBinding, 1> textureBindings{};
    textureBindings[0] = {1, &component.getFontFace().getTexture()};

    vkb.bindDescriptors(shaders.componentWorldText.getPipeline(), shaders.componentWorldText.getDescriptorSetLayout(),
                        bufferBindings, textureBindings);

    ShaderComponentWorldText::Uniforms constants{};
    constants.modelMatrix = transform.getAbsoluteTransform();
    constants.color = component.getColor();
    vkb.pushConstants(shaders.componentWorldText.getPipeline(),
                      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT, 0,
                      sizeof(ShaderComponentWorldText::Uniforms), &constants);

    renderMesh(vkb, mesh);
}

void Renderer::renderSceneCanvas(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene) {
    auto camera = scene.getPrimaryCamera();

    canvas.begin(viewport);

    auto icons = scene.getView<ComponentTransform, ComponentIcon>(entt::exclude<TagDisabled>);
    for (auto&& [entity, transform, component] : icons.each()) {
        const auto pos = camera->worldToScreen(transform.getAbsolutePosition(), true);
        canvas.color(component.getColor());
        canvas.image(pos - component.getSize() / 2.0f, component.getSize(), component.getImage());
    }

    auto texts = scene.getView<ComponentTransform, ComponentText>(entt::exclude<TagDisabled>);
    for (auto&& [entity, transform, component] : texts.each()) {
        component.recalculate(font.regular);

        auto pos = camera->worldToScreen(transform.getAbsolutePosition(), true);
        canvas.color(component.getColor());
        canvas.font(font.regular, static_cast<int>(component.getSize()));

        if (component.getCentered()) {
            pos -= font.regular.getBounds(component.getText(), component.getSize()) / 2.0f;
        }
        canvas.text(pos + component.getOffset(), component.getText());
    }

    canvas.end(vkb);
}

void Renderer::renderNuklear(VulkanCommandBuffer& vkb, const Vector2i& viewport, NuklearWindow& nuklearWindow) {
    canvas.begin(viewport);
    nuklear.begin(viewport);

    nuklearWindow.draw(nuklear, viewport);

    nuklear.end();
    canvas.end(vkb);
}

void Renderer::renderMesh(VulkanCommandBuffer& vkb, const Mesh& mesh) {
    std::array<VulkanVertexBufferBindRef, 1> vboBindings{};
    vboBindings[0] = {&mesh.vbo, 0};

    vkb.bindBuffers(vboBindings);

    if (mesh.ibo) {
        vkb.bindIndexBuffer(mesh.ibo, 0, mesh.indexType);

        vkb.drawIndexed(mesh.count, 1, 0, 0, 0);
    } else {
        vkb.draw(mesh.count, 1, 0, 0);
    }
}

void Renderer::renderLightingPbr(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene,
                                 const Skybox& skybox) {
    updateDirectionalLights(scene);

    auto camera = scene.getPrimaryCamera();

    vkb.bindPipeline(shaders.passPbr.getPipeline());

    std::array<VulkanBufferBinding, 2> bufferBindings{};
    bufferBindings[0] = {0, &camera->getUboZeroPos().getCurrentBuffer()};
    bufferBindings[1] = {1, &directionalLights.ubo.getCurrentBuffer()};

    std::array<VulkanTextureBinding, 8> textureBindings{};
    textureBindings[0] = {2, &textures.pbrAlbedoAmbient};
    textureBindings[1] = {3, &textures.pbrEmissiveRoughness};
    textureBindings[2] = {4, &textures.pbrNormalMetallic};
    textureBindings[3] = {5, &skybox.getIrradiance()};
    textureBindings[4] = {6, &skybox.getPrefilter()};
    textureBindings[5] = {7, &textures.brdf};
    textureBindings[6] = {8, &textures.depth};
    textureBindings[7] = {9, &textures.ssao};

    vkb.bindDescriptors(shaders.passPbr.getPipeline(), shaders.passPbr.getDescriptorSetLayout(), bufferBindings,
                        textureBindings);

    renderMesh(vkb, meshes.fullScreenQuad);
}

void Renderer::updateDirectionalLights(Scene& scene) {
    ShaderPassPbr::DirectionalLightsUniform uniform{};

    auto system = scene.getView<ComponentTransform, ComponentDirectionalLight>();
    for (auto&& [entity, transform, light] : system.each()) {
        uniform.colors[uniform.count] = light.getColor();
        uniform.directions[uniform.count] = Vector4{transform.getPosition(), 0.0f};

        uniform.count++;
        if (uniform.count >= ShaderPassPbr::maxDirectionalLights) {
            break;
        }
    }

    if (!directionalLights.ubo) {
        VulkanBuffer::CreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeof(ShaderPassPbr::DirectionalLightsUniform);
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
        bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

        directionalLights.ubo = vulkan.createDoubleBuffer(bufferInfo);
    }

    directionalLights.ubo.subDataLocal(&uniform, 0, sizeof(uniform));
}
