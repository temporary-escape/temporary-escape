#include "render_pass.hpp"

using namespace Engine;

RenderPass::RenderPass(VulkanRenderer& vulkan, const Vector2i& viewport) : vulkan{vulkan}, viewport{viewport} {
}

VkFormat RenderPass::findDepthFormat() {
    return vulkan.findSupportedFormat(
        {
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT,
        },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void RenderPass::addAttachment(const AttachmentInfo& attachmentInfo, const VkImageLayout initialLayout,
                               const VkImageLayout finalLayout, const VkAttachmentLoadOp loadOp) {
    auto format = attachmentInfo.format;
    if (isDepthFormat(format)) {
        depthIndex = textures.size();
    }

    VulkanTexture::CreateInfo textureInfo{};
    textureInfo.image.format = format;
    textureInfo.image.imageType = VK_IMAGE_TYPE_2D;
    textureInfo.image.extent = {static_cast<uint32_t>(viewport.x), static_cast<uint32_t>(viewport.y), 1};
    textureInfo.image.mipLevels = 1;
    textureInfo.image.arrayLayers = 1;
    textureInfo.image.tiling = VK_IMAGE_TILING_OPTIMAL;
    textureInfo.image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    textureInfo.image.usage = attachmentInfo.usage | VK_IMAGE_USAGE_SAMPLED_BIT;
    textureInfo.image.samples = VK_SAMPLE_COUNT_1_BIT;
    textureInfo.image.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    textureInfo.view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    textureInfo.view.format = format;
    textureInfo.view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    textureInfo.view.subresourceRange.aspectMask = attachmentInfo.aspectMask; // VK_IMAGE_ASPECT_COLOR_BIT;
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

    textures.push_back(vulkan.createTexture(textureInfo));
    attachmentViews.push_back(textures.back().getImageView());
    attachments.push_back(&textures.back());

    VkAttachmentDescription attachmentDescription{};
    attachmentDescription.format = textures.back().getFormat();
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = loadOp;
    attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescription.stencilLoadOp = loadOp;
    attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescription.initialLayout = initialLayout; // VK_IMAGE_LAYOUT_UNDEFINED
    attachmentDescription.finalLayout = finalLayout;
    attachmentDescriptions.push_back(attachmentDescription);
}

void RenderPass::addAttachment(const VulkanTexture& texture, const VkImageLayout initialLayout,
                               const VkImageLayout finalLayout, const VkAttachmentLoadOp loadOp,
                               const VkAttachmentLoadOp stencilOp) {
    attachmentViews.push_back(texture.getImageView());
    attachments.push_back(&texture);

    if (isDepthFormat(texture.getFormat())) {
        depthIndex = attachmentDescriptions.size();
    }

    VkAttachmentDescription attachmentDescription{};
    attachmentDescription.format = texture.getFormat();
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = loadOp;
    attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescription.stencilLoadOp = stencilOp;
    attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    attachmentDescription.initialLayout = initialLayout;
    attachmentDescription.finalLayout = finalLayout;
    attachmentDescriptions.push_back(attachmentDescription);
}

void RenderPass::init(const bool compute) {
    if (!compute) {
        for (auto& subpass : subpasses) {
            addSubpass(subpass->getAttachments(), subpass->getInputs());
        }
        createRenderPass();
        createFbo();

        uint32_t index{0};
        for (auto& subpass : subpasses) {
            subpass->init(renderPass, index++);
        }
    } else {
        for (auto& subpass : subpasses) {
            subpass->init();
        }
    }
}

void RenderPass::createFbo() {
    VulkanFramebuffer::CreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass.getHandle();
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentViews.size());
    framebufferInfo.pAttachments = attachmentViews.data();
    framebufferInfo.width = viewport.x;
    framebufferInfo.height = viewport.y;
    framebufferInfo.layers = 1;

    fbo = vulkan.createFramebuffer(framebufferInfo);
}

void RenderPass::addSubpass(const Span<uint32_t>& indexes, const Span<uint32_t>& inputs) {
    subpassDescriptionData.emplace_back();
    auto& data = subpassDescriptionData.back();

    VkSubpassDescription subpassDescription{};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    for (const auto& index : indexes) {
        if (index >= attachments.size()) {
            EXCEPTION("Subpass attachment index: {} out of range", index);
        }

        if (index == depthIndex) {
            data.depthReference = {index, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
            subpassDescription.pDepthStencilAttachment = &data.depthReference;
        } else {
            data.colorReferences.push_back({index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
        }
    }

    for (const auto& index : inputs) {
        if (index >= attachments.size()) {
            EXCEPTION("Subpass input index: {} out of range", index);
        }

        data.inputsReferences.push_back({index, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
    }

    if (!data.colorReferences.empty()) {
        subpassDescription.colorAttachmentCount = static_cast<uint32_t>(data.colorReferences.size());
        subpassDescription.pColorAttachments = data.colorReferences.data();
    }

    if (!data.inputsReferences.empty()) {
        subpassDescription.inputAttachmentCount = static_cast<uint32_t>(data.inputsReferences.size());
        subpassDescription.pInputAttachments = data.inputsReferences.data();
    }

    if (subpassDescriptions.empty() && depthIndex != UINT64_MAX) {
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask =
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependency.dstStageMask =
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        dependency.dstAccessMask =
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies.push_back(dependency);
    }

    VkSubpassDependency dependency{};

    if (subpassDescriptions.empty()) {
        // First subpass
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    } else {
        // Additional subpass
        dependency.srcSubpass = subpassDescriptions.size() - 1;
        dependency.dstSubpass = subpassDescriptions.size();
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    }

    dependencies.push_back(dependency);

    subpassDescriptions.push_back(subpassDescription);
}

void RenderPass::createRenderPass() {
    if (!subpassDescriptions.empty()) {
        VkSubpassDependency dependency{};
        dependency.srcSubpass = subpassDescriptions.size() - 1;
        dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies.push_back(dependency);
    }

    VulkanRenderPass::CreateInfo renderPassInfo = {};
    renderPassInfo.attachments = attachmentDescriptions;
    renderPassInfo.dependencies = dependencies;
    renderPassInfo.subPasses = subpassDescriptions;

    renderPass = vulkan.createRenderPass(renderPassInfo);
}

void RenderPass::transitionRead(VulkanCommandBuffer& vkb, const VulkanTexture& texture) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkb.start(beginInfo);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    if (isDepthFormat(texture.getFormat())) {
        barrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    } else {
        barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = texture.getHandle();
    if (isDepthFormat(texture.getFormat())) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = texture.getMipMaps();
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    vkb.pipelineBarrier(sourceStage, destinationStage, barrier);
}

void RenderPass::transitionWrite(VulkanCommandBuffer& vkb, const VulkanTexture& texture) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkb.start(beginInfo);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    if (isDepthFormat(texture.getFormat())) {
        barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    } else {
        barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = texture.getHandle();
    if (isDepthFormat(texture.getFormat())) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = texture.getMipMaps();
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;

    VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    vkb.pipelineBarrier(sourceStage, destinationStage, barrier);
}
