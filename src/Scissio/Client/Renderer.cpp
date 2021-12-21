#include "Renderer.hpp"

using namespace Scissio;

Renderer::Renderer(Canvas2D& canvas, const Config& config, AssetManager& assetManager, Client& client)
    : canvas(canvas), gui(canvas, config, assetManager), client(client),
      widgetDebugStats(gui, client.getStats()), shaders{ShaderModel{config}} {
}

Renderer::~Renderer() = default;

void Renderer::render(const Vector2i& viewport) {
    const auto t0 = std::chrono::steady_clock::now();

    canvas.beginFrame(viewport);
    gui.reset();

    widgetDebugStats.render();

    gui.render(viewport);
    canvas.endFrame();

    const auto t1 = std::chrono::steady_clock::now();
    const auto tDiff = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
    client.getStats().render.frameTimeMs.store(tDiff.count());
}

void Renderer::renderScene(Scene& scene) {
}

void Renderer::renderComponentModel(ComponentModel& component) {
}
