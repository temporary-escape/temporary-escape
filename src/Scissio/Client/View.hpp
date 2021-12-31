#pragma once

#include "Renderer.hpp"
#include "WidgetDebugStats.hpp"

namespace Scissio {
class Client;

class View {
public:
    explicit View(const Config& config, Canvas2D& canvas, AssetManager& assetManager, Renderer& renderer,
                  Client& client);

    void render(const Vector2i& viewport);
    void eventMouseMoved(const Vector2i& pos);
    void eventMousePressed(const Vector2i& pos, MouseButton button);
    void eventMouseReleased(const Vector2i& pos, MouseButton button);
    void eventMouseScroll(int xscroll, int yscroll);
    void eventKeyPressed(Key key, Modifiers modifiers);
    void eventKeyReleased(Key key, Modifiers modifiers);

private:
    const Config& config;
    Canvas2D& canvas;
    AssetManager& assetManager;
    Renderer& renderer;
    Client& client;
    GuiContext gui;

    struct CameraData {
        bool move[6] = {false};
        Vector2 rotation{0.0f};
        bool rotate{false};
        Vector2 mousePosOld;
    } cameraData;

    struct WidgetsData {
        WidgetDebugStats debugStats;
    } widgets;
};
} // namespace Scissio
