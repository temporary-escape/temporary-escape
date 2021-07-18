#pragma once

#include "../Assets/AssetManager.hpp"
#include "../Graphics/Renderer.hpp"
#include "../Graphics/SkyboxRenderer.hpp"
#include "../Graphics/ThumbnailRenderer.hpp"
#include "../Math/Vector.hpp"
#include "../Network/NetworkClient.hpp"
#include "../Platform/Enums.hpp"
#include "../Scene/Scene.hpp"
#include "../Utils/Worker.hpp"
#include "Messages.hpp"
#include "Store.hpp"
#include "ViewBuild.hpp"
#include "ViewMap.hpp"
#include "ViewSpace.hpp"

namespace Scissio {
class SCISSIO_API AbstractClient : public Network::Client {
public:
    explicit AbstractClient(const Config& config, AssetManager& assetManager, const std::string& address, uint64_t uid);
    virtual ~AbstractClient() = default;

    void update();
    void dispatch(Network::Packet packet) override;

protected:
    virtual void onSectorLoaded() = 0;

    const Config& config;
    AssetManager& assetManager;
    Store store;
    std::unique_ptr<Scene> scene;
    Network::MessageDispatcher<> dispatcher;
    asio::io_service worker;

private:
    void syncHello();
    void handle(MessageHelloResponse message);
    void handle(MessageSectorChanged message);
    void handle(MessageSectorStatusResponse message);
    void handle(MessageEntityBatch message);
};

class SCISSIO_API Client : public AbstractClient {
public:
    explicit Client(const Config& config, AssetManager& assetManager, Renderer& renderer,
                    SkyboxRenderer& skyboxRenderer, ThumbnailRenderer& thumbnailRenderer, Canvas2D& canvas,
                    const std::string& address, uint64_t uid);
    ~Client() override = default;

    void render(GBuffer& gBuffer, FBuffer& fbuffer, const Vector2i& viewport);
    void eventMouseMoved(const Vector2i& pos);
    void eventMousePressed(const Vector2i& pos, MouseButton button);
    void eventMouseReleased(const Vector2i& pos, MouseButton button);
    void eventKeyPressed(Key key, Modifiers modifiers);
    void eventKeyReleased(Key key, Modifiers modifiers);
    void eventMouseScroll(int xscroll, int yscroll);

protected:
    void onSectorLoaded() override;

private:
    void handle(MessageSystemsResponse message);
    void handle(MessageRegionsResponse message);
    void handle(MessageBlocksResponse message);

    Renderer& renderer;
    SkyboxRenderer& skyboxRenderer;
    ThumbnailRenderer& thumbnailRenderer;
    Canvas2D& canvas;
    GuiContext gui;
    std::unique_ptr<ViewSpace> viewSpace;
    std::unique_ptr<ViewBuild> viewBuild;
    std::unique_ptr<ViewMap> viewMap;
    View* view;
    Skybox skybox;
    Vector2i mousePos;
};
} // namespace Scissio
