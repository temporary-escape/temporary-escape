#pragma once

#include "../Graphics/Canvas2D.hpp"
#include "Client.hpp"
#include "WidgetDebugStats.hpp"

namespace Scissio {
class Renderer {
public:
    Renderer(Canvas2D& canvas, const Config& config, AssetManager& assetManager, Client& client);
    ~Renderer();

    void render(const Vector2i& viewport);

private:
    Canvas2D& canvas;
    GuiContext gui;
    Client& client;
    WidgetDebugStats widgetDebugStats;
};
} // namespace Scissio