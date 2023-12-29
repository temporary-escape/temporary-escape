#pragma once

#include "../Graphics/RendererCanvas.hpp"
#include "Windows/GuiWindowContextMenu.hpp"
#include "Windows/GuiWindowModal.hpp"

namespace Engine {
class ENGINE_API GuiManager {
public:
    using ModalCallback = std::function<void(bool)>;

    explicit GuiManager(VulkanRenderer& vulkan, RendererCanvas& renderer, const FontFamily& fontFamily, int fontSize);
    virtual ~GuiManager();

    void render(VulkanCommandBuffer& vkb, const Vector2i& viewport);
    void blit(Canvas& canvas);

    template <typename T, typename... Args> T* addWindow(Args&&... args) {
        windows.emplace_back();
        windows.back().canvas = std::make_unique<Canvas>(vulkan);
        windows.back().ptr = std::make_shared<T>(fontFamily, fontSize, std::forward<Args>(args)...);
        return dynamic_cast<T*>(windows.back().ptr.get());
    }
    void removeWindow(const GuiWindow& window);
    void setFocused(const GuiWindow& window);
    void clearFocused();

    void clearContextMenu();
    void addContextMenuItem(std::string label, ImagePtr icon, GuiWidgetButton::OnClickCallback onClick);
    void showContextMenu(const Vector2i& pos);
    void hideContextMenu();
    bool isContextMenuVisible() const;

    GuiWindowModal* modalSuccess(std::string title, std::string text, const ModalCallback& callback = nullptr);
    GuiWindowModal* modalDanger(std::string title, std::string text, const ModalCallback& callback = nullptr);
    GuiWindowModal* modal(std::string title, std::string text, const std::vector<std::string>& choices,
                          const ModalCallback& callback = nullptr);

    bool isMousePosOverlap(const Vector2i& mousePos) const;

    void eventMouseMoved(const Vector2i& pos);
    void eventMousePressed(const Vector2i& pos, MouseButton button);
    void eventMouseReleased(const Vector2i& pos, MouseButton button);
    void eventMouseScroll(int xscroll, int yscroll);
    void eventKeyPressed(Key key, Modifiers modifiers);
    void eventKeyReleased(Key key, Modifiers modifiers);
    void eventCharTyped(uint32_t code);

private:
    struct WindowData {
        std::unique_ptr<Canvas> canvas;
        std::shared_ptr<GuiWindow> ptr;
        VulkanTexture fboColor;
        VulkanFramebuffer fbo;
        bool hover{false};
        uint64_t buttonMask{0};
    };

    void removeWindowInternal(const GuiWindow* window);
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

    std::list<WindowData> windows;
    std::vector<const GuiWindow*> windowsToRemove;
    const GuiWindow* focused{nullptr};
    GuiWindowContextMenu* contextMenu;
};
} // namespace Engine
