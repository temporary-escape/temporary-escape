#pragma once

#include "../Graphics/RendererCanvas.hpp"
#include "GuiWindow.hpp"

namespace Engine {
class GuiManager {
public:
    explicit GuiManager(VulkanRenderer& vulkan, RendererCanvas& renderer, const FontFamily& fontFamily, int fontSize);

    void render(VulkanCommandBuffer& vkb, const Vector2i& viewport);
    void blit(Canvas2& canvas);

    template <typename T, typename... Args> std::shared_ptr<T> addWindow(Args&&... args) {
        windows.emplace_back();
        windows.back().canvas = std::make_unique<Canvas2>(vulkan);
        windows.back().ptr = std::make_shared<T>(fontFamily, fontSize, std::forward<Args>(args)...);
        return std::dynamic_pointer_cast<T>(windows.back().ptr);
    }

    void eventMouseMoved(const Vector2i& pos);
    void eventMousePressed(const Vector2i& pos, MouseButton button);
    void eventMouseReleased(const Vector2i& pos, MouseButton button);
    void eventMouseScroll(int xscroll, int yscroll);
    void eventKeyPressed(Key key, Modifiers modifiers);
    void eventKeyReleased(Key key, Modifiers modifiers);
    void eventCharTyped(uint32_t code);

private:
    struct WindowData {
        std::unique_ptr<Canvas2> canvas;
        std::shared_ptr<GuiWindow2> ptr;
        VulkanTexture fboColor;
        VulkanFramebuffer fbo;
        bool input{false};
        uint64_t buttonMask{0};
    };

    void createRenderPass();
    void createFbo(WindowData& data);
    void createFboTexture(WindowData& data);
    bool fboNeedsResizing(WindowData& data) const;
    VkExtent3D getExtent(WindowData& data) const;

    VulkanRenderer& vulkan;
    RendererCanvas& renderer;
    const FontFamily& fontFamily;
    int fontSize;
    VulkanRenderPass renderPass;

    std::vector<WindowData> windows;
};
} // namespace Engine
