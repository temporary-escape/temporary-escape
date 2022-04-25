#pragma once

#include "../Audio/AudioContext.hpp"
#include "../Future.hpp"
#include "../Modding/ModManager.hpp"
#include "../Platform/OpenGLWindow.hpp"
#include "../Server/Server.hpp"
#include "../Utils/RocksDB.hpp"
#include "Client.hpp"
#include "GBuffer.hpp"
#include "Imager.hpp"
#include "Renderer.hpp"
#include "Shaders.hpp"
#include "ViewBuild.hpp"
#include "ViewMap.hpp"
#include "ViewSpace.hpp"
#include "ViewVoxelTest.hpp"
#include <atomic>

namespace Engine {
class ENGINE_API Application : public OpenGLWindow {
public:
    Application(Config& config);
    virtual ~Application();

    void update();
    void render(const Vector2i& viewport) override;
    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;

    void load();

private:
    void renderView(const Vector2i& viewport);

    Config& config;

    Canvas2D canvas;
    Canvas2D::FontHandle defaultFont;

    Future<void> stagesFuture;
    asio::io_service worker;

    GBuffer gBuffer;

    std::shared_ptr<AudioContext> audioContext;
    std::shared_ptr<TextureCompressor> textureCompressor;
    std::shared_ptr<SkyboxRenderer> skyboxRenderer;
    std::shared_ptr<AssetManager> assetManager;
    std::shared_ptr<Grid::Builder> gridBuilder;
    std::shared_ptr<ModManager> modManager;
    std::shared_ptr<TransactionalDatabase> db;
    std::shared_ptr<Stats> stats;
    std::shared_ptr<Server> server;
    std::shared_ptr<Client> client;
    std::shared_ptr<Shaders> shaders;
    std::shared_ptr<GBuffer> gbuffer;
    std::shared_ptr<Renderer> renderer;
    std::shared_ptr<Imager> imager;
    std::shared_ptr<GuiContext> gui;
    std::shared_ptr<Widgets> widgets;
    std::shared_ptr<ViewSpace> viewSpace;
    std::shared_ptr<ViewMap> viewMap;
    std::shared_ptr<ViewVoxelTest> viewVoxelTest;
    std::shared_ptr<ViewBuild> viewBuild;
    View* view{nullptr};

    std::atomic<bool> loading;
    std::atomic<float> loadingProgress;
};
} // namespace Engine
