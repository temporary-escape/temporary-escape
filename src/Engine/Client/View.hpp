#pragma once

#include "../Graphics/Nuklear.hpp"
#include "../Graphics/Renderer.hpp"
#include "../Window.hpp"

namespace Engine {
class ENGINE_API Renderer;
class ENGINE_API AssetsManager;
class ENGINE_API GuiManager;
class ENGINE_API RendererSkybox;
class ENGINE_API RendererPlanetSurface;
class ENGINE_API RendererBackground;
class ENGINE_API Canvas2;

class ENGINE_API View : public UserInput {
public:
    virtual ~View() = default;

    virtual void update(float deltaTime, const Vector2i& viewport) = 0;
    virtual void renderCanvas(Canvas2& canvas, const Vector2i& viewport) = 0;
    virtual void onEnter() = 0;
    virtual void onExit() = 0;
    virtual Scene* getScene() = 0;
};

class ENGINE_API ViewContext : public UserInput {
public:
    ViewContext(const Config& config, VulkanRenderer& vulkan, AssetsManager& assetsManager, GuiManager& guiManager,
                RendererBackground& rendererBackground);

    template <typename T, typename... Args> T* addView(Args&&... args) {
        auto ptr = new T(config, vulkan, guiManager, std::forward<Args>(args)...);
        views.emplace_back(ptr);
        return ptr;
    }

    void setCurrent(View* value);
    void clear();
    [[nodiscard]] View* getCurrent() const {
        return current;
    }

    void update(float deltaTime, const Vector2i& viewport);
    void render(VulkanCommandBuffer& vkb, Renderer& renderer, const Vector2i& viewport);
    void renderCanvas(Canvas2& canvas, const Vector2i& viewport);

    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;
    void eventCharTyped(uint32_t code) override;

private:
    const Config& config;
    VulkanRenderer& vulkan;
    AssetsManager& assetsManager;
    GuiManager& guiManager;
    RendererBackground& rendererBackground;

    View* current;
    std::list<std::unique_ptr<View>> views;
};
} // namespace Engine
