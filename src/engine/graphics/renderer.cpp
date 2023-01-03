#include "renderer.hpp"
#include "../utils/exceptions.hpp"

#define CMP "Renderer"

using namespace Engine;

Renderer::ShaderLoadQueue Renderer::Shaders::createLoadQueue() {
    return {
        [this](const Config& config, VulkanRenderer& renderer) {
            brdf = ShaderBrdf{config, renderer};
        },
        [this](const Config& config, VulkanRenderer& renderer) {
            componentGrid = ShaderComponentGrid{config, renderer};
        },
    };
}

Renderer::Renderer(const Config& config, VulkanRenderer& vulkan, Shaders& shaders, VoxelShapeCache& voxelShapeCache) :
    config{config},
    vulkan{vulkan},
    shaders{shaders},
    voxelShapeCache{voxelShapeCache},
    lastViewportSize{config.windowWidth, config.windowHeight} {

    // createGaussianKernel(15, 6.5);
    // createSsaoNoise();
    // createSsaoSamples();
    // createFullScreenQuad();
    // createSkyboxMesh();

    // createRenderPass(renderPasses.brdf, {config.brdfSize, config.brdfSize});

    createMeshes();
    createRenderPasses();
    finalizeShaders();

    renderBrdf();
}

Renderer::~Renderer() = default;

VulkanBuffer Renderer::createUniformBuffer(const size_t size) {
    VulkanBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage =
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    return vulkan.createBuffer(bufferInfo);
}

VulkanBuffer Renderer::createVertexBuffer(const size_t size) {
    VulkanBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage =
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    return vulkan.createBuffer(bufferInfo);
}

VulkanBuffer Renderer::createIndexBuffer(const size_t size) {
    VulkanBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage =
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    return vulkan.createBuffer(bufferInfo);
}

void Renderer::createMeshes() {
    createFullScreenQuad();
}

void Renderer::createFullScreenQuad() {
    static const std::vector<ShaderBrdf::Vertex> vertices = {
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
    bufferInfo.size = sizeof(ShaderBrdf::Vertex) * vertices.size();
    bufferInfo.usage =
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

    meshes.fullScreenQuad.vbo = vulkan.createBuffer(bufferInfo);
    vulkan.copyDataToBuffer(meshes.fullScreenQuad.vbo, vertices.data(), sizeof(ShaderBrdf::Vertex) * vertices.size());

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

void Renderer::finalizeShaders() {
    shaders.brdf.finalize(renderPasses.brdf.renderPass);
    shaders.componentGrid.finalize(renderPasses.pbr.renderPass);
}

void Renderer::createRenderPasses() {
    // vulkan.waitDeviceIdle();
    if (!renderPasses.brdf.renderPass) {
        createRenderPassBrdf();
    }
    createDepthTexture();
    createRenderPassPbr();
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
    renderPasses.depthFormat = findDepthFormat();

    createAttachment(
        // Render pass
        renderPasses.depth,
        // Size
        lastViewportSize,
        // Format
        renderPasses.depthFormat,
        // Usage
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        // Aspect mask
        VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    vulkan.transitionImageLayout(renderPasses.depth, VK_IMAGE_LAYOUT_UNDEFINED,
                                 VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void Renderer::createRenderPassBrdf() {
    renderPasses.brdf.textures.emplace_back();
    createAttachment(
        // Render pass
        renderPasses.brdf.textures.back(),
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

    VkImageView attachments[] = {renderPasses.brdf.textures.at(0).getImageView()};

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
    renderPasses.pbr.textures.emplace_back();
    createAttachment(
        // Render pass
        renderPasses.pbr.textures.back(),
        // Size
        lastViewportSize,
        // Format
        VK_FORMAT_R8G8B8A8_UNORM,
        // Usage
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        // Aspect mask
        VK_IMAGE_ASPECT_COLOR_BIT);

    // Emissive + Roughness
    renderPasses.pbr.textures.emplace_back();
    createAttachment(
        // Render pass
        renderPasses.pbr.textures.back(),
        // Size
        lastViewportSize,
        // Format
        VK_FORMAT_R8G8B8A8_UNORM,
        // Usage
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        // Aspect mask
        VK_IMAGE_ASPECT_COLOR_BIT);

    // Normal + Metallic
    renderPasses.pbr.textures.emplace_back();
    createAttachment(
        // Render pass
        renderPasses.pbr.textures.back(),
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

    attachments[3].format = renderPasses.depthFormat;
    attachments[3].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[3].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[3].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

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
        renderPasses.pbr.textures.at(0).getImageView(),
        renderPasses.pbr.textures.at(1).getImageView(),
        renderPasses.pbr.textures.at(2).getImageView(),
        renderPasses.depth.getImageView(),
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
    textureInfo.image.usage =
        usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
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

/*void Renderer::update(const Vector2i& viewport) {
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
    }
}*/

void Renderer::render(const Vector2i& viewport, Scene& scene, Skybox& skybox, const Options& options) {
    if (viewport != lastViewportSize) {
        lastViewportSize = viewport;
        vulkan.waitDeviceIdle();
        createRenderPasses();
    }

    auto camera = scene.getPrimaryCamera();
    if (!camera) {
        EXCEPTION("Scene has no camera");
    }
    camera->recalculate(vulkan, viewport);

    renderPassPbr(viewport, scene, options);

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

    /*canvas.begin(viewport);
    renderStatus(viewport);
    canvas.end(vkb);*/

    vkb.endRenderPass();
    vkb.end();

    // vulkan.submitPresentCommandBuffer(vkb);
    vulkan.submitCommandBuffer(vkb, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, renderPasses.pbr.semaphore,
                               vulkan.getCurrentRenderFinishedSemaphore(), &vulkan.getCurrentInFlightFence());
    vulkan.dispose(std::move(vkb));

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

    vulkan.submitCommandBuffer(vkb, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               vulkan.getCurrentImageAvailableSemaphore(), renderPasses.pbr.semaphore, nullptr);
    vulkan.dispose(std::move(vkb));
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
    auto& systemGrids = scene.getComponentSystem<ComponentGrid>();
    auto camera = scene.getPrimaryCamera();

    std::array<VulkanBufferBinding, 2> bufferBindings{};
    std::array<VulkanTextureBinding, 5> textureBindings{};
    std::array<VulkanVertexBufferBindRef, 1> vboBindings{};

    bufferBindings[0] = {0, &camera->getUbo().getCurrentBuffer()};

    vkb.bindPipeline(shaders.componentGrid.getPipeline());

    for (auto* component : systemGrids) {
        component->recalculate(vulkan, voxelShapeCache);

        const Matrix4 transform = component->getObject().getAbsoluteTransform();
        const Matrix3 transformInverted = glm::transpose(glm::inverse(glm::mat3x3(transform)));

        const auto& primitives = component->getPrimitives();
        ShaderComponentGrid::Uniforms uniforms{};
        uniforms.modelMatrix = transform;
        uniforms.normalMatrix = transformInverted;

        vkb.pushConstants(shaders.componentGrid.getPipeline(),
                          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT, 0,
                          sizeof(ShaderComponentGrid::Uniforms), &uniforms);

        for (auto& primitive : primitives) {
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

/*void Renderer::renderPassFront(bool clear) {
    VulkanFramebufferAttachmentReference frontColorAttachment{};
    frontColorAttachment.clearValue.color = {0.0f, 0.0f, 0.0f, 0.0f};
    if (clear) {
        frontColorAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
    } else {
        frontColorAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;
    }

    vulkan.beginRenderPass(gBuffer.frontFbo, {frontColorAttachment});
}*/

/*void Renderer::present() {
    // vulkan.submitPresentQueue(gBuffer.frontColor);
}*/

/*void Renderer::createGaussianKernel(const size_t size, double sigma) {
    const auto weights = gaussianKernel((size - 1) * 2 + 1, sigma);
    GaussianWeightsUniform data;

    for (size_t i = 0; i < size; i++) {
        const auto w = static_cast<float>(weights[size - i - 1]);
        data.weight[i] = Vector4{w, w, w, 1.0f};
    }
    data.count = static_cast<int>(size);

    gaussianWeights.ubo =
        vulkan.createBuffer(VulkanBuffer::Type::Uniform, VulkanBuffer::Usage::Static, sizeof(GaussianWeightsUniform));
    gaussianWeights.ubo.subData(&data, 0, sizeof(GaussianWeightsUniform));
}*/

/*void Renderer::createSsaoNoise() {
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
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
    ssaoSamples.noise.subData(0, {0, 0}, desc.size, ssaoNoise.data());
}*/

/*void Renderer::createFullScreenQuad() {
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
}*/

/*void Renderer::renderFullScreenQuad() {
    vulkan.setDepthStencilState(false, false);
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
    vulkan.drawIndexed(6, 1, 0, 0, 0);
}*/

/*void Renderer::createSkyboxMesh() {
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
}*/

/*void Renderer::renderSkyboxMesh() {
    vulkan.setDepthStencilState(false, true);
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
    vulkan.drawIndexed(6 * 6, 1, 0, 0, 0);
}*/

/*void Renderer::updateDirectionalLightsUniform(Scene& scene) {
    DirectionalLightsUniform uniform{};

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
    }
}*/

/*void Renderer::createSsaoSamples() {
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
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
    ssaoSamples.ubo.subData(&uniform, 0, sizeof(SsaoSamplesUniform));
}*/
