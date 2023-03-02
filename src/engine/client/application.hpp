#pragma once

#include "../database/database.hpp"
#include "../graphics/renderer.hpp"
#include "../graphics/skybox_generator.hpp"
#include "../server/server.hpp"
#include "../vulkan/vulkan_renderer.hpp"
#include "game.hpp"
#include "gui/gui_main_menu.hpp"
#include <queue>

namespace Engine {
struct ENGINE_API Status {
    std::string message{""};
    float value{0.0f};
};

class ENGINE_API Application : public VulkanRenderer {
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
    void checkForClientScene();
    void loadAssets();
    void createRegistry();
    void createVoxelShapeCache();
    void createRenderer();
    void loadNextAssetInQueue(Registry::LoadQueue::const_iterator next);
    void compileShaders();
    void compileNextShaderInQueue(ShaderModules::LoadQueue::iterator next);
    void startDatabase();
    void startServer();
    void startClient();
    void loadProfile();
    void startSinglePlayer();

    const Config& config;

    Canvas canvas;
    FontFamily font;
    Nuklear nuklear;
    Status status;

    struct {
        GuiMainMenu mainMenu;
    } gui;

    std::unique_ptr<Registry> registry;
    std::unique_ptr<Game> game;
    std::unique_ptr<TransactionalDatabase> db;
    std::unique_ptr<Server::Certs> serverCerts;
    std::unique_ptr<Server> server;
    std::thread serverThread;
    std::unique_ptr<ShaderModules> shaderModules;
    std::unique_ptr<SkyboxGenerator> skyboxGenerator;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<VoxelShapeCache> voxelShapeCache;
    std::unique_ptr<Client> client;
    PlayerLocalProfile playerLocalProfile;

    std::future<std::function<void()>> future;
    std::promise<std::function<void()>> promise;
    std::atomic<bool> shouldStop{false};
    Vector2i mousePos;
};
} // namespace Engine
