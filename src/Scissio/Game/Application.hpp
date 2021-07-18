#pragma once

#include "../Assets/AssetManager.hpp"
#include "../Assets/FontFace.hpp"
#include "../Config.hpp"
#include "../Future.hpp"
#include "../Graphics/Canvas2D.hpp"
#include "../Graphics/Renderer.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/SkyboxRenderer.hpp"
#include "../Graphics/TextureCompressor.hpp"
#include "../Graphics/ThumbnailRenderer.hpp"
#include "../Modding/ModManager.hpp"
#include "../Platform/OpenGLWindow.hpp"
#include "../Utils/Database.hpp"

namespace Scissio {
class Client;
class Server;

class SCISSIO_API Application : public OpenGLWindow {
public:
    enum class State : uint64_t {
        Init = 1 << 0,
        LoadAssets = 1 << 1,
        LoadServer = 1 << 2,
        WaitServer = 1 << 3,
        LoadClient = 1 << 4,
        WaitClient = 1 << 5,
        Playing = 1 << 6,
    };

    explicit Application(const Config& config);
    virtual ~Application();

    void render(const Vector2i& viewport) override;
    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;
    void eventMouseScroll(int xscroll, int yscroll) override;

private:
    uint64_t generatePlayerUid();

    State state = State::Init;

    const Config& config;
    Canvas2D canvas;
    Canvas2D::FontHandle defaultFont;
    uint64_t uid;
    std::unique_ptr<ModManager> modManager;
    std::unique_ptr<TextureCompressor> textureCompressor;
    std::unique_ptr<SkyboxRenderer> skyboxRenderer;
    std::unique_ptr<ThumbnailRenderer> thumbnailRenderer;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<GBuffer> gBuffer;
    std::unique_ptr<FBuffer> fBuffer;
    std::unique_ptr<AssetManager> assetManager;
    std::unique_ptr<Database> database;
    std::unique_ptr<Server> server;
    std::unique_ptr<Client> client;
    Future<void> clientFuture;
    Future<void> serverFuture;
};
} // namespace Scissio
