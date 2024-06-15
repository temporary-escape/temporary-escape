#pragma once

#include "../Graphics/RendererCanvas.hpp"
#include "Windows/GuiWindowContextMenu.hpp"
#include "Windows/GuiWindowModal.hpp"

namespace Engine {
class ENGINE_API GuiManager {
public:
    using ModalCallback = std::function<bool(const std::string&)>;
    using ContextMenuCallback = std::function<void(GuiWindowContextMenu&)>;

    explicit GuiManager(const Config& config, VulkanRenderer& vulkan, const FontFamily& fontFamily, int fontSize);
    virtual ~GuiManager();

    void draw(const Vector2i& viewport);
    void render(VulkanCommandBuffer& vkb, RendererCanvas& renderer, const Vector2i& viewport);

    template <typename T, typename... Args> T* addWindow(Args&&... args) {
        auto ptr = std::make_shared<T>(ctx, fontFamily, fontSize, std::forward<Args>(args)...);
        windows.push_back(ptr);
        return ptr.get();
    }
    void removeWindow(const GuiWindow& window);
    void setFocused(const GuiWindow& window);
    void clearFocused();

    bool isContextMenuVisible() const;

    GuiWindowModal* modal(std::string title, std::string text);
    void showModal(GuiWindowModal& window);
    void closeModal(GuiWindowModal& window);
    GuiWindowContextMenu& getContextMenu() {
        return *contextMenu;
    }
    void showContextMenu(const Vector2& pos, ContextMenuCallback callback);

    bool isMousePosOverlap(const Vector2i& mousePos) const;

    void eventMouseMoved(const Vector2i& pos);
    void eventMousePressed(const Vector2i& pos, MouseButton button);
    void eventMouseReleased(const Vector2i& pos, MouseButton button);
    void eventMouseScroll(int xscroll, int yscroll);
    void eventKeyPressed(Key key, Modifiers modifiers);
    void eventKeyReleased(Key key, Modifiers modifiers);
    void eventCharTyped(uint32_t code);
    void eventInputBegin();
    void eventInputEnd();

private:
    void removeWindowInternal(const GuiWindow* window);
    Vector2i getScaledViewport(const Vector2i& viewport) const;

    const Config& config;
    VulkanRenderer& vulkan;
    const FontFamily& fontFamily;
    int fontSize;
    GuiContext ctx;
    VulkanRenderPass renderPass;
    Canvas canvas;

    std::list<std::shared_ptr<GuiWindow>> windows;
    std::vector<const GuiWindow*> windowsToRemove;
    const GuiWindow* focused{nullptr};
    GuiWindowContextMenu* contextMenu;
};
} // namespace Engine
