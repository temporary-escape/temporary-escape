#include "Client.hpp"
#include "../Assets/AssetManager.hpp"
//#include "../Network/NetworkTcpClient.hpp"
#include "../Scene/ComponentModel.hpp"
#include "../Server/Messages.hpp"
#include "../Utils/Random.hpp"
#include <fstream>

#define CMP "Client"

/*#define DISPATCH_FUNC(M, T, F) std::bind(static_cast<void (T::*)(M)>(&T::F), this, std::placeholders::_1)
#define MESSAGE_DISPATCH(M) dispatcher.add<M>(DISPATCH_FUNC(M, Client, handle));
#define MESSAGE_DISPATCH_FETCH(M)                                                                                      \
    dispatcher.add<M::Response>(std::bind(&Client::handleFetch<M>, this, std::placeholders::_1));*/

using namespace Engine;

std::atomic<uint64_t> Client::nextRequestId(1);

Client::Client(Config& config, const std::string& address, const int port) : requestsNextId(1), sync(getWorker()) {

    /*MESSAGE_DISPATCH(MessageServerHello);
    MESSAGE_DISPATCH(MessageLoginResponse);
    MESSAGE_DISPATCH(MessageStatusResponse);
    MESSAGE_DISPATCH(MessageSectorChanged);
    MESSAGE_DISPATCH(MessageEntitySync);
    MESSAGE_DISPATCH(MessageEntityDeltas);
    MESSAGE_DISPATCH_FETCH(MessageFetchGalaxy);
    MESSAGE_DISPATCH_FETCH(MessageFetchGalaxySystems);
    MESSAGE_DISPATCH_FETCH(MessageFetchGalaxyRegions);
    MESSAGE_DISPATCH_FETCH(MessageFetchCurrentLocation);
    MESSAGE_DISPATCH_FETCH(MessageFetchSystemPlanets);*/

    const auto profilePath = config.userdataPath / Path("profile.xml");
    if (Fs::exists(profilePath)) {
        Xml::loadAsXml(profilePath, localProfile);
    } else {
        localProfile.secret = randomId();
        localProfile.name = "Some Player";
        Xml::saveAsXml(profilePath, "profile", localProfile);
    }

    connect(address, port);

    MessageLogin::Request req{};
    req.secret = localProfile.secret;
    req.name = localProfile.name;
    send(req);

    auto future = loggedIn.future();
    auto t0 = std::chrono::steady_clock::now();
    while (const auto status = future.waitFor(std::chrono::milliseconds(10)) != std::future_status::ready) {
        sync.restart();
        sync.run();

        auto now = std::chrono::steady_clock::now();
        if (now - t0 > std::chrono::milliseconds(1000)) {
            NetworkTcpClient<ServerSink>::stop();
            EXCEPTION("Timeout while waiting for login");
        }
    }
    future.get();

    // network = std::make_shared<Network::TcpClient>(*listener, address, port);

    /*auto future = connected.future();
    if (future.waitFor(std::chrono::milliseconds(1000)) != std::future_status::ready) {
        EXCEPTION("Timeout while waiting for server connection");
    }
    future.get();*/

    /*MessageStatusRequest req{};
    req.timePoint = std::chrono::system_clock::now();
    send(req);*/
}

Client::~Client() {
    NetworkTcpClient<ServerSink>::stop();
}

/*void Client::eventPacket(Network::Packet packet) {
    try {
        stats.network.packetsReceived++;
        dispatcher.dispatch(packet);
    } catch (...) {
        EXCEPTION_NESTED("Failed to dispatch message");
    }
}

void Client::eventConnect() {
}

void Client::eventDisconnect() {
}*/

void Client::update() {
    sync.restart();
    sync.run();

    if (scene) {
        const auto now = std::chrono::steady_clock::now();
        auto timeDiff = now - lastTimePoint;
        lastTimePoint = now;
        if (timeDiff > std::chrono::milliseconds(100)) {
            timeDiff = std::chrono::milliseconds(100);
        }
        const auto delta = std::chrono::duration_cast<std::chrono::microseconds>(timeDiff).count() / 1000000.0f;
        scene->update(delta);
    }
}

void Client::handle(MessageLogin::Response res) {
    if (!res.error.empty()) {
        loggedIn.reject<std::runtime_error>(res.error);
    } else {
        playerId = res.playerId;
        loggedIn.resolve();
    }
}

void Client::handle(MessagePlayerLocation::Response res) {
    Log::i(CMP, "Sector has changed, creating new scene");

    camera.reset();
    scene.reset();
    scene = std::make_unique<Scene>();

    camera = std::make_shared<Entity>();
    auto cmp = camera->addComponent<ComponentCameraTurntable>();
    auto userInput = camera->addComponent<ComponentUserInput>(*cmp);
    cmp->setPrimary(true);
    cmp->setProjection(70.0f);
    scene->addEntity(camera);

    store.player.location = res.location;
    store.player.location.markChanged();
}

void Client::handle(MessageSceneEntities::Response res) {
    try {
        if (!scene) {
            EXCEPTION("Client scene is not initialized");
        }
        for (auto& proxy : res.entities) {
            const auto entity = std::dynamic_pointer_cast<Entity>(proxy);
            if (!entity) {
                EXCEPTION("Failed to cast EntityProxy to Entity");
            }
            scene->addEntity(entity);

            if (auto cmp = entity->findComponent<ComponentPlayer>(); cmp != nullptr && cmp->getPlayerId() == playerId) {
                camera->getComponent<ComponentCameraTurntable>()->follow(entity);
            }
        }
    } catch (...) {
        EXCEPTION_NESTED("Failed to process scene entities");
    }
}

void Client::handle(MessageSceneDeltas::Response res) {
    try {
        if (!scene) {
            EXCEPTION("Client scene is not initialized");
        }
        for (auto& delta : res.deltas) {
            scene->updateEntity(delta);
        }
    } catch (...) {
        EXCEPTION_NESTED("Failed to process scene entities");
    }
}

void Client::handle(MessageFetchGalaxy::Response res) {
    store.galaxy.galaxy = std::move(res.galaxy);
    store.galaxy.galaxy.markChanged();
}

void Client::handle(MessageFetchRegions::Response res) {
    store.galaxy.regions.value().clear();
    for (auto& item : res.regions) {
        store.galaxy.regions.value().insert(std::make_pair(item.id, std::move(item)));
    }

    store.galaxy.regions.markChanged();
}

void Client::handle(MessageFetchSystems::Response res) {
    store.galaxy.systems.value().clear();
    for (auto& item : res.systems) {
        store.galaxy.systems.value().insert(std::make_pair(item.id, std::move(item)));
    }

    store.galaxy.systems.markChanged();
}

void Client::handle(MessageShipMovement::Response res) {
}
