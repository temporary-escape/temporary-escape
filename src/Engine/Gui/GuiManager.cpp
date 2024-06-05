#include "GuiManager.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

GuiManager::GuiManager(const Config& config, VulkanRenderer& vulkan, const FontFamily& fontFamily, const int fontSize) :
    config{config},
    vulkan{vulkan},
    fontFamily{fontFamily},
    fontSize{fontSize},
    canvas{vulkan},
    ctx{fontFamily, fontSize} {

    auto nestedContextMenu = addWindow<GuiWindowContextMenu>(nullptr);
    contextMenu = addWindow<GuiWindowContextMenu>(nestedContextMenu);
}

GuiManager::~GuiManager() = default;

void GuiManager::draw(const Vector2i& viewport) {
    const auto scaledViewport = getScaledViewport(viewport);

    for (const auto window : windowsToRemove) {
        removeWindowInternal(window);
    }
    windowsToRemove.clear();

    ctx.update();

    for (auto& window : windows) {
        window->update(scaledViewport);

        if (!window->isEnabled()) {
            if (focused == window.get()) {
                clearFocused();
            }
            continue;
        }

        if (window.get() == focused) {
            ctx.setFocused(window->getId());
            ctx.setInputEnabled(true);
        } else if (focused) {
            ctx.setInputEnabled(false);
        } else {
            ctx.setInputEnabled(true);
        }

        window->draw();
    }

    canvas.begin(scaledViewport);
    ctx.render(canvas);
    canvas.flush();
}

Vector2i GuiManager::getScaledViewport(const Vector2i& viewport) const {
    return Vector2i{Vector2{viewport} * config.gui.scale};
}

void GuiManager::render(VulkanCommandBuffer& vkb, RendererCanvas& renderer, const Vector2i& viewport) {
    const auto scaledViewport = getScaledViewport(viewport);

    renderer.render(vkb, canvas, scaledViewport);
}

void GuiManager::removeWindowInternal(const GuiWindow* window) {
    auto it = std::find_if(windows.begin(), windows.end(), [&window](const std::shared_ptr<GuiWindow>& ptr) {
        return ptr.get() == window;
    });

    if (window == focused) {
        clearFocused();
    }

    if (it != windows.end()) {
        windows.erase(it);
    }
}

void GuiManager::removeWindow(const GuiWindow& window) {
    windowsToRemove.push_back(&window);
}

void GuiManager::setFocused(const GuiWindow& window) {
    auto it = std::find_if(windows.begin(), windows.end(), [&window](const std::shared_ptr<GuiWindow>& ptr) {
        return ptr.get() == &window;
    });

    if (it == windows.end()) {
        return;
    }

    focused = &window;
}

void GuiManager::clearFocused() {
    focused = nullptr;
}

bool GuiManager::isContextMenuVisible() const {
    return contextMenu->isEnabled();
}

GuiWindowModal* GuiManager::modal(std::string title, std::string text, const std::vector<std::string>& choices,
                                  const ModalCallback& callback, const int timeout) {
    auto* window = addWindow<GuiWindowModal>(std::move(title), std::move(text), choices, timeout);
    showModal(*window);
    window->setOnClickCallback([this, window, callback](const std::string& choice) {
        if (!callback || callback(choice)) {
            this->closeModal(*window);
        }
    });
    return window;
}

GuiWindowModal* GuiManager::modalPrimary(std::string title, std::string text, const ModalCallback& callback) {
    static std::vector<std::string> choices{"Ok"};
    auto window = modal(std::move(title), std::move(text), choices, callback);
    window->setHeaderPrimary(true);
    return window;
}

GuiWindowModal* GuiManager::modalSuccess(std::string title, std::string text, const ModalCallback& callback) {
    static std::vector<std::string> choices{"Ok"};
    auto window = modal(std::move(title), std::move(text), choices, callback);
    window->setHeaderSuccess(true);
    return window;
}

GuiWindowModal* GuiManager::modalDanger(std::string title, std::string text, const ModalCallback& callback) {
    static std::vector<std::string> choices{"Ok"};
    auto window = modal(std::move(title), std::move(text), choices, callback);
    window->setHeaderDanger(true);
    return window;
}

void GuiManager::showModal(GuiWindowModal& window) {
    window.setEnabled(true);
    setFocused(window);
}

void GuiManager::closeModal(GuiWindowModal& window) {
    if (window.isEnabled()) {
        window.setEnabled(false);
        clearFocused();
        removeWindow(window);
    }
}

void GuiManager::showContextMenu(const Vector2& pos, ContextMenuCallback callback) {
    contextMenu->clear();
    callback(*contextMenu);
    contextMenu->setEnabled(true);
    contextMenu->setPos(pos * config.gui.scale);
}

bool GuiManager::isMousePosOverlap(const Vector2i& mousePos) const {
    const auto mouse = Vector2{mousePos} * config.gui.scale;

    for (auto& window : windows) {
        if (!window->isEnabled()) {
            continue;
        }

        const auto& pos = window->getPos();
        const auto& size = window->getSize();

        if (mouse.x >= pos.x && mouse.x <= pos.x + size.x && mouse.y >= pos.y && mouse.y <= pos.y + size.y) {
            return true;
        }
    }

    return false;
}

void GuiManager::eventMouseMoved(const Vector2i& pos) {
    ctx.eventMouseMoved(Vector2{pos} * config.gui.scale);
}

void GuiManager::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    ctx.eventMousePressed(Vector2{pos} * config.gui.scale, button);
}

void GuiManager::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    ctx.eventMouseReleased(Vector2{pos} * config.gui.scale, button);
}

void GuiManager::eventMouseScroll(const int xscroll, const int yscroll) {
    ctx.eventMouseScroll(xscroll, yscroll);
}

void GuiManager::eventKeyPressed(const Key key, const Modifiers modifiers) {
    ctx.eventKeyPressed(key, modifiers);
}

void GuiManager::eventKeyReleased(const Key key, const Modifiers modifiers) {
    ctx.eventKeyReleased(key, modifiers);
}

void GuiManager::eventCharTyped(const uint32_t code) {
    ctx.eventCharTyped(code);
}

void GuiManager::eventInputBegin() {
    ctx.eventInputBegin();
}

void GuiManager::eventInputEnd() {
    ctx.eventInputEnd();
}

/*void GuiManager::createRenderPass() {
    VkAttachmentDescription attachmentDescription{};
    attachmentDescription.format = vulkan.getSwapChain().getFormat();
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescription.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescription.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescription.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDescription.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

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
}*/

/*void GuiManager::createFbo(GuiManager::WindowData& data) {
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
}*/

/*void GuiManager::createFboTexture(GuiManager::WindowData& data) {
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
}*/

/*bool GuiManager::fboNeedsResizing(GuiManager::WindowData& data) const {
    const auto expected = getExtent(data);
    return !data.fbo || !data.fboColor || data.fboColor.getExtent().width != expected.width ||
           data.fboColor.getExtent().height != expected.height;
}*/

/*VkExtent3D GuiManager::getExtent(GuiManager::WindowData& data) const {
    const auto size = data.ptr->getSize();
    const auto width = (static_cast<uint32_t>(std::floor(size.x / 128.0f)) + 1) * 128;
    const auto height = (static_cast<uint32_t>(std::floor(size.y / 128.0f)) + 1) * 128;
    return {width, height, 1};
}*/
