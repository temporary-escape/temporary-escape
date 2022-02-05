#pragma once

#include "../Future.hpp"
#include "../Modding/ModManager.hpp"
#include "../Platform/OpenGLWindow.hpp"
#include "../Server/Server.hpp"
#include "../Audio/AudioContext.hpp"
#include "Client.hpp"
#include "Renderer.hpp"
#include "ViewRoot.hpp"
#include <atomic>

namespace Engine {
class ENGINE_API Application : public OpenGLWindow {
public:
    struct Options {
        std::optional<std::string> saveFolderName;
        bool saveFolderClean{false};
    };

    Application(Config& config, const Options& options);
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
    const Options& options;

    Canvas2D canvas;
    Canvas2D::FontHandle defaultFont;

    Future<void> stagesFuture;
    Promise<AssetLoadQueue> loadQueuePromise;
    Future<AssetLoadQueue> loadQueueFuture;
    AssetLoadQueue assetLoadQueue;
    Promise<bool> assetLoadQueueFinished;

    std::shared_ptr<AudioContext> audioContext;
    std::shared_ptr<TextureCompressor> textureCompressor;
    std::shared_ptr<AssetManager> assetManager;
    std::shared_ptr<ModManager> modManager;
    std::shared_ptr<Database> db;
    std::shared_ptr<Server> server;
    std::shared_ptr<Client> client;
    std::shared_ptr<Renderer> renderer;
    std::shared_ptr<ViewRoot> view;

    std::atomic<size_t> assetLoadQueueInitialSize;
    std::atomic<bool> loading;
    std::atomic<float> loadingProgress;
};
} // namespace Engine
