#include "RenderPassSsao.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Controllers/ControllerLights.hpp"
#include "../../Scene/Scene.hpp"

using namespace Engine;

RenderPassSSAO::RenderPassSSAO(const RenderOptions& options, VulkanRenderer& vulkan, RenderBufferPbr& buffer,
                               RenderResources& resources) :
    RenderPass{vulkan, buffer, "RenderPassSSAO"},
    options{options},
    buffer{buffer},
    resources{resources},
    pipelineSSAO{vulkan} {

    { // SSAO
        AttachmentInfo attachment{};
        attachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment.clearColor = {1.0f, 1.0f, 1.0f, 1.0f};
        addAttachment(RenderBufferPbr::Attachment::SSAO, attachment);
    }

    addSubpass(
        {
            RenderBufferPbr::Attachment::SSAO,
        },
        {});

    { // Dependency for Depth
        DependencyInfo dependency{};
        dependency.srcStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependency.dstStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependency.srcAccessMask = VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependency.dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;
        addSubpassDependency(dependency);
    }

    addPipeline(pipelineSSAO, 0);
}

void RenderPassSSAO::beforeRender(VulkanCommandBuffer& vkb) {
    {
        // Depth
        /*const auto& texture = buffer.getAttachmentTexture(RenderBufferPbr::Attachment::Depth);
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = texture.getHandle();
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = texture.getMipMaps();
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = texture.getLayerCount();
        barrier.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        barrier.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;

        vkb.pipelineBarrier(VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                            VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                            barrier);*/
    }

    { // Normal
        const auto& texture = buffer.getAttachmentTexture(RenderBufferPbr::Attachment::NormalMetallic);
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = texture.getHandle();
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = texture.getMipMaps();
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = texture.getLayerCount();
        barrier.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;

        vkb.pipelineBarrier(VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                            VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                            barrier);
    }
}

void RenderPassSSAO::render(VulkanCommandBuffer& vkb, Scene& scene) {
    auto& camera = *scene.getPrimaryCamera();

    vkb.setViewport({0, 0}, getViewport());
    vkb.setScissor({0, 0}, getViewport());

    pipelineSSAO.bind(vkb);

    const auto scale = Vector2{getViewport()} / 4.0f;
    pipelineSSAO.setKernelSize(options.ssao);
    pipelineSSAO.setScale(scale);
    pipelineSSAO.flushConstants(vkb);

    pipelineSSAO.setUniformCamera(camera.getUbo().getCurrentBuffer());
    pipelineSSAO.setUniformSamples(resources.getSSAOKernel());
    pipelineSSAO.setTextureNoise(resources.getSSAONoise());
    pipelineSSAO.setTextureDepth(buffer.getAttachmentTexture(RenderBufferPbr::Attachment::Depth));
    pipelineSSAO.setTextureNormal(buffer.getAttachmentTexture(RenderBufferPbr::Attachment::NormalMetallic));
    pipelineSSAO.flushDescriptors(vkb);

    pipelineSSAO.renderMesh(vkb, resources.getMeshFullScreenQuad());
}
