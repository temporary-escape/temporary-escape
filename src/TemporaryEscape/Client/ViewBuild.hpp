#pragma once

#include "../Assets/AssetImage.hpp"
#include "../Gui/GuiContext.hpp"
#include "../Server/Schemas.hpp"
#include "View.hpp"
#include "Widgets.hpp"

namespace Engine {
class ENGINE_API ViewBuild : public View {
public:
    explicit ViewBuild(const Config& config, Canvas2D& canvas, AssetManager& assetManager, Renderer& renderer,
                       Widgets& widgets);

    void load();
    void render(const Vector2i& viewport) override;
    void renderGui(const Vector2i& viewport) override;
    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;

private:
    const Config& config;
    Canvas2D& canvas;
    AssetManager& assetManager;
    Renderer& renderer;
    Widgets& widgets;

    std::unique_ptr<Scene> scene;
};
} // namespace Engine
