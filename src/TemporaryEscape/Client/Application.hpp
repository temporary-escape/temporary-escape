#pragma once

#include "../Vulkan/VulkanWindow.hpp"
#include "Game.hpp"
#include "Renderer.hpp"
#include <queue>

namespace Engine {
class Application : public VulkanWindow {
public:
    explicit Application(const Config& config);
    virtual ~Application();

    void render(const Vector2i& viewport, float deltaTime) override;

    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;
    void eventWindowResized(const Vector2i& size) override;

private:
    void renderStatus(const Vector2i& viewport);

    const Config& config;
    Renderer::Pipelines rendererPipelines;
    Scene::Pipelines scenePipelines;
    Renderer renderer;
    Canvas canvas;
    FontFamily font;
    Status status;

    std::unique_ptr<Registry> registry;
    std::unique_ptr<Game> game;

    std::queue<std::function<void()>> shaderQueue;
    bool loadShaders{false};
    Future<void> futureRegistry;
    bool loadRegistry{false};
    bool loadAssets{false};
    bool loadGame{false};
};
} // namespace Engine
