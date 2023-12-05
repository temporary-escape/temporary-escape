#include "GuiManager.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

GuiManager::GuiManager(VulkanRenderer& vulkan, RendererCanvas& renderer, const FontFamily& fontFamily,
                       const int fontSize) :
    vulkan{vulkan}, renderer{renderer}, fontFamily{fontFamily}, fontSize{fontSize} {
    createRenderPass();
}

void GuiManager::render(VulkanCommandBuffer& vkb, const Vector2i& viewport) {
    for (auto& window : windows) {
        window.ptr->update(viewport);

        if (!window.ptr->isDirty()) {
            continue;
        }

        const auto extent = getExtent(window);

        window.ptr->draw();

        window.canvas->begin({extent.width, extent.height});
        window.ptr->render(*window.canvas);
        window.canvas->flush();
    }

    for (auto& window : windows) {
        if (!window.ptr->isDirty()) {
            continue;
        }

        const auto extent = getExtent(window);

        if (fboNeedsResizing(window)) {
            logger.info("Creating FBO of size: {} for gui window: \"{}\"",
                        Vector2i{extent.width, extent.height},
                        window.ptr->getTitle());

            createFboTexture(window);
            createFbo(window);
        }

        VulkanRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.framebuffer = &window.fbo;
        renderPassInfo.renderPass = &renderPass;
        renderPassInfo.offset = {0, 0};
        renderPassInfo.size = {extent.width, extent.height};

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 0.0f}}};
        renderPassInfo.clearValues = {clearColor};

        vkb.beginRenderPass(renderPassInfo);
        vkb.setViewport({0, 0}, renderPassInfo.size);
        // vkb.setScissor({0, 0}, renderPassInfo.size);

        renderer.render(vkb, *window.canvas, renderPassInfo.size);

        vkb.endRenderPass();

        window.ptr->clearDirty();
    }
}

void GuiManager::blit(Canvas2& canvas) {
    for (auto& window : windows) {
        canvas.drawTexture(window.ptr->getPos(), window.ptr->getSize(), window.fboColor, {1.0f, 1.0f, 1.0f, 1.0f});
    }
}

void GuiManager::eventMouseMoved(const Vector2i& pos) {
    for (auto& window : windows) {
        const auto& p = Vector2i{window.ptr->getPos()};
        const auto& s = Vector2i{window.ptr->getSize()};

        if (p.x <= pos.x && p.x + s.x >= pos.x && p.y <= pos.y && p.y + s.y >= pos.y) {
            window.input = true;
            window.ptr->eventMouseMoved(pos - p);
        } else if (window.input) {
            window.input = false;
            window.ptr->eventMouseMoved({999999.0f, 999999.0f});
        }
    }
}

void GuiManager::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    for (auto& window : windows) {
        const auto& p = Vector2i{window.ptr->getPos()};
        const auto& s = Vector2i{window.ptr->getSize()};

        if (p.x <= pos.x && p.x + s.x >= pos.x && p.y <= pos.y && p.y + s.y >= pos.y) {
            window.buttonMask |= 1 << static_cast<uint64_t>(button);
            window.ptr->eventMousePressed(pos - p, button);
        }
    }
}

void GuiManager::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    for (auto& window : windows) {
        const auto& p = Vector2i{window.ptr->getPos()};
        const auto& s = Vector2i{window.ptr->getSize()};

        if ((p.x <= pos.x && p.x + s.x >= pos.x && p.y <= pos.y && p.y + s.y >= pos.y) ||
            (window.buttonMask & 1 << static_cast<uint64_t>(button))) {
            window.ptr->eventMouseReleased(pos - p, button);
        }

        window.buttonMask &= ~(1 << static_cast<uint64_t>(button));
    }
}

void GuiManager::eventMouseScroll(const int xscroll, const int yscroll) {
    for (auto& window : windows) {
        if (window.input) {
            window.ptr->eventMouseScroll(xscroll, yscroll);
        }
    }
}

void GuiManager::eventKeyPressed(const Key key, const Modifiers modifiers) {
}

void GuiManager::eventKeyReleased(const Key key, const Modifiers modifiers) {
}

void GuiManager::eventCharTyped(const uint32_t code) {
}

void GuiManager::createRenderPass() {
    VkAttachmentDescription attachmentDescription{};
    attachmentDescription.format = vulkan.getSwapChain().getFormat();
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescription.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescription.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescription.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDescription.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference colorAttachment{};
    colorAttachment.attachment = 0;
    colorAttachment.layout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription{};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorAttachment;

    VulkanRenderPass::CreateInfo renderPassInfo = {};
    renderPassInfo.attachments = {attachmentDescription};
    renderPassInfo.dependencies = {};
    renderPassInfo.subPasses = {subpassDescription};

    renderPass = vulkan.createRenderPass(renderPassInfo);
}

void GuiManager::createFbo(GuiManager::WindowData& data) {
    if (data.fbo) {
        vulkan.dispose(std::move(data.fbo));
    }

    const auto extent = getExtent(data);

    VulkanFramebuffer::CreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = vulkan.getRenderPass().getHandle();
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = &data.fboColor.getImageView();
    framebufferInfo.width = extent.width;
    framebufferInfo.height = extent.height;
    framebufferInfo.layers = 1;

    data.fbo = vulkan.createFramebuffer(framebufferInfo);
}

void GuiManager::createFboTexture(GuiManager::WindowData& data) {
    if (data.fboColor) {
        vulkan.dispose(std::move(data.fboColor));
    }

    const auto extent = getExtent(data);

    VulkanTexture::CreateInfo textureInfo{};
    textureInfo.image.format = vulkan.getSwapChain().getFormat();
    textureInfo.image.imageType = VkImageType::VK_IMAGE_TYPE_2D;
    textureInfo.image.extent = extent;
    textureInfo.image.mipLevels = 1;
    textureInfo.image.arrayLayers = 1;
    textureInfo.image.tiling = VK_IMAGE_TILING_OPTIMAL;
    textureInfo.image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    textureInfo.image.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    textureInfo.image.samples = VK_SAMPLE_COUNT_1_BIT;
    textureInfo.image.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    textureInfo.view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    textureInfo.view.format = textureInfo.image.format;
    textureInfo.view.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
    textureInfo.view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    textureInfo.view.subresourceRange.baseMipLevel = 0;
    textureInfo.view.subresourceRange.levelCount = 1;
    textureInfo.view.subresourceRange.baseArrayLayer = 0;
    textureInfo.view.subresourceRange.layerCount = 1;

    textureInfo.sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    textureInfo.sampler.magFilter = VK_FILTER_LINEAR;
    textureInfo.sampler.minFilter = VK_FILTER_LINEAR;
    textureInfo.sampler.addressModeU = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    textureInfo.sampler.addressModeV = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    textureInfo.sampler.addressModeW = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    textureInfo.sampler.anisotropyEnable = VK_FALSE;
    textureInfo.sampler.maxAnisotropy = 1.0f;
    textureInfo.sampler.borderColor = VkBorderColor::VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    textureInfo.sampler.unnormalizedCoordinates = VK_FALSE;
    textureInfo.sampler.compareEnable = VK_FALSE;
    textureInfo.sampler.compareOp = VK_COMPARE_OP_ALWAYS;
    textureInfo.sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    textureInfo.sampler.minLod = 0.0f;
    textureInfo.sampler.maxLod = 0.0f;

    data.fboColor = vulkan.createTexture(textureInfo);
}

bool GuiManager::fboNeedsResizing(GuiManager::WindowData& data) const {
    const auto expected = getExtent(data);
    return !data.fbo || !data.fboColor || data.fboColor.getExtent().width != expected.width ||
           data.fboColor.getExtent().height != expected.height;
}

VkExtent3D GuiManager::getExtent(GuiManager::WindowData& data) const {
    const auto size = data.ptr->getSize();
    const auto width = (static_cast<uint32_t>(std::floor(size.x / 128.0f)) + 1) * 128;
    const auto height = (static_cast<uint32_t>(std::floor(size.y / 128.0f)) + 1) * 128;
    return {width, height, 1};
}
