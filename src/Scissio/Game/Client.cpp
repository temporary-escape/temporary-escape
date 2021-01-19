#include "Client.hpp"

#include "../Assets/Block.hpp"
#include "../Assets/Model.hpp"
#include "../Network/NetworkTcpConnector.hpp"
#include "../Network/NetworkUdpConnector.hpp"
#include "../Scene/ComponentCamera.hpp"
#include "../Scene/ComponentGrid.hpp"
#include "../Scene/ComponentModel.hpp"

using namespace Scissio;

Client::Client(const Config& config, EventBus& eventBus, AssetManager& assetManager, Renderer& renderer,
               SkyboxRenderer& skyboxRenderer, Canvas2D& canvas, const std::string& address)
    : Network::Client(0, address), config(config), eventBus(eventBus), assetManager(assetManager), renderer(renderer),
      skyboxRenderer(skyboxRenderer), canvas(canvas), view(nullptr), renderMode(0) {

    initConnection<Network::TcpConnector>(config.serverPort, 1234, "admin");
    addConnection<Network::UdpConnector>(config.serverPort);
    syncHello();

    ADD_EVENT_LISTENER(eventBus, EventBuildMode, Client, handle);
    ADD_EVENT_LISTENER(eventBus, EventSpaceMode, Client, handle);
}

void Client::syncHello() {
    MessageClientHello hello{};
    hello.version = {1, 0, 0};
    send(0, hello);

    const auto res = response.get<MessageServerHello>();
    Log::v("Client received hello for server: {}", res.name);
}

void Client::initScene() {
    scene = std::make_unique<Scene>();
    viewSpace = std::make_unique<ViewSpace>(config, eventBus, renderer, *scene);
    view = viewSpace.get();

    skybox = skyboxRenderer.renderAndFilter(7418525);

    std::mt19937_64 rng;
    for (auto x = 0; x < 8; x++) {
        for (auto y = 0; y < 8; y++) {
            auto model = assetManager.find<Model>("model_asteroid_01_a");
            auto entity = scene->addEntity();
            entity->addComponent<ComponentModel>(model);
            entity->translate({x * 3.0f, 0.0f, y * 3.0f});
            entity->rotate(randomQuaternion(rng));
        }
    }

    /*auto model = assetManager.find<Model>("model_block_01_cube");
    auto entity = scene->addEntity();
    entity->addComponent<ComponentModel>(model);
    entity->translate({-3.0f, 0.0f, 0.0f});*/

    auto block = assetManager.find<Block>("block_hull_01_cube");
    auto entity = scene->addEntity();
    auto grid = entity->addComponent<ComponentGrid>();
    grid->insert(block, {0, 0, 0}, 0);
    grid->insert(block, {1, 0, 0}, 0);
    grid->insert(block, {2, 0, 0}, 0);
    // entity->translate({0.0f, 0.0f, 0.0f});

    entity = scene->addEntity();
    entity->addComponent<ComponentCamera>();
    entity->translate({0.0f, 0.0f, 2.0f});
}

void Client::render(GBuffer& gBuffer, const Vector2i& viewport) {
    if (scene) {
        gBuffer.bind(viewport);

        renderer.setViewport(viewport);
        view->render(viewport);

        gBuffer.unbind();

        renderer.renderSkybox(skybox.texture);
        renderer.renderPbr(gBuffer, skybox);

        canvas.beginFrame(viewport);
        view->canvas(viewport);
        canvas.endFrame();
    } else {
        initScene();
    }
}

void Client::handle(MessageServerHello message) {
    response.store(std::move(message));
}

void Client::handle(const EventBuildMode& event) {
    viewBuild = std::make_unique<ViewBuild>(config, eventBus, assetManager, renderer, canvas);
    view = viewBuild.get();
}

void Client::handle(const EventSpaceMode& event) {
    viewBuild.reset();
    view = viewSpace.get();
}

#define MESSAGE_DISPATCH(T)                                                                                            \
    case T::KIND: {                                                                                                    \
        this->handle(Scissio::Network::Details::unpack<T>(packet.data));                                               \
        break;                                                                                                         \
    }

void Client::dispatch(const Network::Packet& packet) {
    switch (packet.id) {
        MESSAGE_DISPATCH(MessageServerHello)
    default: {
        Log::e("Client cannot accept unknown packet id: {}", packet.id);
    }
    }
}

void Client::eventMouseMoved(const Vector2i& pos) {
    if (view) {
        view->eventMouseMoved(pos);
    }
}

void Client::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    if (view) {
        view->eventMousePressed(pos, button);
    }
}

void Client::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    if (view) {
        view->eventMouseReleased(pos, button);
    }
}

void Client::eventKeyPressed(const Key key) {
    if (view) {
        view->eventKeyPressed(key);
    }
}

void Client::eventKeyReleased(const Key key) {
    if (view) {
        view->eventKeyReleased(key);
    }
}
