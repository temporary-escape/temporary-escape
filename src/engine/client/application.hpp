#pragma once

#include "../database/database.hpp"
#include "../graphics/renderer.hpp"
#include "../graphics/skybox_generator.hpp"
#include "../gui/gui_main_menu.hpp"
#include "../server/server.hpp"
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
    void createRegistry();
    void loadNextAssetInQueue(Registry::LoadQueue::const_iterator next);
    void compileShaders();
    void compileNextShaderInQueue(Renderer::ShaderLoadQueue::iterator next);
    void startDatabase();
    void startServer();
    void startClient();
    void loadServer();
    void startSinglePlayer();

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
    Renderer::Shaders shaders;
    Renderer::ShaderLoadQueue shaderLoadQueue;

    struct {
        GuiMainMenu mainMenu;
    } gui;

    std::unique_ptr<Registry> registry;
    std::unique_ptr<Game> game;
    std::unique_ptr<TransactionalDatabase> db;
    std::unique_ptr<Server::Certs> serverCerts;
    std::unique_ptr<Server> server;

    std::future<std::function<void()>> future;
    std::promise<std::function<void()>> promise;
    std::atomic<bool> shouldStop{false};
};
} // namespace Engine
