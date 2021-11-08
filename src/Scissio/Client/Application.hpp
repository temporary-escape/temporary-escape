#pragma once

#include "../Future.hpp"
#include "../Modding/ModManager.hpp"
#include "../Platform/Window.hpp"
#include "../Server/Generator.hpp"
#include "../Server/Server.hpp"
#include "Client.hpp"
#include <atomic>

namespace Scissio {
class SCISSIO_API Application : public Window {
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
    Config& config;

    Canvas2D canvas;
    Canvas2D::FontHandle defaultFont;

    Future<void> stagesFuture;
    Promise<AssetLoadQueue> loadQueuePromise;
    Future<AssetLoadQueue> loadQueueFuture;
    AssetLoadQueue assetLoadQueue;
    Promise<bool> assetLoadQueueFinished;

    std::shared_ptr<TextureCompressor> textureCompressor;
    std::shared_ptr<AssetManager> assetManager;
    std::shared_ptr<ModManager> modManager;
    std::shared_ptr<Database> db;
    std::shared_ptr<Generator> generator;
    std::shared_ptr<Server> server;
    std::shared_ptr<Client> client;

    std::atomic<size_t> assetLoadQueueInitialSize;
    std::atomic<bool> loading;
    std::atomic<float> loadingProgress;
};
} // namespace Scissio
