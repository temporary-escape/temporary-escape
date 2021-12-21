#pragma once

#include "../Graphics/Canvas2D.hpp"
#include "../Scene/ComponentModel.hpp"
#include "../Scene/Scene.hpp"
#include "../Shaders/ShaderModel.hpp"
#include "Client.hpp"
#include "WidgetDebugStats.hpp"

namespace Scissio {
class Renderer {
public:
    Renderer(Canvas2D& canvas, const Config& config, AssetManager& assetManager, Client& client);
    ~Renderer();

    void render(const Vector2i& viewport);

private:
    void renderScene(Scene& scene);
    void renderComponentModel(ComponentModel& component);

    Canvas2D& canvas;
    GuiContext gui;
    Client& client;
    WidgetDebugStats widgetDebugStats;

    struct Shaders {
        ShaderModel model;
    } shaders;
};
} // namespace Scissio
