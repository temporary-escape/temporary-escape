#pragma once

#include "../graphics/renderer.hpp"
#include "../graphics/skybox_generator.hpp"
#include "../gui/gui_main_menu.hpp"
#include "../utils/async_loader.hpp"
#include "../vulkan/vulkan_renderer.hpp"
#include "game.hpp"
#include <queue>

namespace Engine {
class Application : public VulkanRenderer {
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
    void eventCharTyped(uint32_t code) override;
    void eventWindowBlur() override;
    void eventWindowFocus() override;

private:
    void renderStatus(const Vector2i& viewport);

    const Config& config;
    // Renderer::Pipelines rendererPipelines;
    // SkyboxGenerator::Pipelines skyboxGeneratorPipelines;
    // Scene::Pipelines scenePipelines;
    Renderer renderer;
    SkyboxGenerator skyboxGenerator;
    Canvas canvas;
    FontFamily font;
    Nuklear nuklear;
    Status status;
    AsyncLoader asyncLoader;

    struct {
        GuiMainMenu mainMenu;
    } gui;

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
