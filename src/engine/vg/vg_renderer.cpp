#include "vg_renderer.hpp"
#include "../utils/exceptions.hpp"
#include "../utils/log.hpp"

#define CMP "VgRenderer"

using namespace Engine;

VgRenderer::VgRenderer(const Config& config) :
    VgDevice{config},
    VgCommandBuffer{getCommandPool().createCommandBuffer()},
    lastViewportSize{config.windowWidth, config.windowHeight} {

    createRenderPass();
    createSwapChainFramebuffers();
}

void VgRenderer::createRenderPass() {
    VgRenderPass::CreateInfo renderPassInfo{};

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = getSwapChain().getFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    renderPassInfo.attachments = {colorAttachment};

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    renderPassInfo.subPasses = {subpass};

    renderPass = VgDevice::createRenderPass(renderPassInfo);
}

void VgRenderer::createSwapChainFramebuffers() {
    const auto& swapChainImageViews = getSwapChain().getImageViews();

    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        VkImageView attachments[] = {swapChainImageViews[i]};

        VgFramebuffer::CreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass.getHandle();
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = getSwapChain().getExtent().width;
        framebufferInfo.height = getSwapChain().getExtent().height;
        framebufferInfo.layers = 1;

        swapChainFramebuffers[i] = createFramebuffer(framebufferInfo);
    }
}

void VgRenderer::render(const Vector2i& viewport, float deltaTime) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    startCommandBuffer(beginInfo);

    draw(viewport, deltaTime);

    endCommandBuffer();
    submitCommandBuffer(*this);

    submitPresentQueue();

    // Do we need to recreate the swap chain?
    if (lastViewportSize != viewport) {
        recreateSwapChain();
    }

    lastViewportSize = viewport;
}

VgPipeline VgRenderer::createPipeline(const VgPipeline::CreateInfo& createInfo) {
    return VgDevice::createPipeline(renderPass, createInfo);
}

VgFramebuffer& VgRenderer::getSwapChainFramebuffer() {
    return swapChainFramebuffers.at(getSwapChainFramebufferIndex());
}

void VgRenderer::beginRenderPass(const VgFramebuffer& framebuffer, const Vector2i& size) {
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass.getHandle();
    renderPassInfo.framebuffer = framebuffer.getHandle();
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = VkExtent2D{static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y)};

    VkClearValue clearColor = {{{0.3f, 0.3f, 0.3f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    VgCommandBuffer::beginRenderPass(renderPassInfo);
}

void VgRenderer::onSwapChainChanged() {
    createSwapChainFramebuffers();
}
