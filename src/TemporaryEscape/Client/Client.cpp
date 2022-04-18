#include "Client.hpp"
#include "../Utils/Random.hpp"
#include <fstream>

#define CMP "Client"

using namespace Engine;

Client::Client(const Config& config, Stats& stats, Store& store, const std::string& address, const int port)
    : stats(stats), store(store), sync(getWorker()) {

    const auto profilePath = config.userdataPath / Path("profile.yml");
    if (Fs::exists(profilePath)) {
        localProfile.fromYaml(profilePath);
    } else {
        localProfile.secret = randomId();
        localProfile.name = "Some Player";
        localProfile.toYaml(profilePath);
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
}

Client::~Client() {
    NetworkTcpClient<ServerSink>::stop();
}

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

        store.player.id = playerId;
        store.player.id.markChanged();
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
    cmp->setProjection(70.0f);
    scene->addEntity(camera);
    scene->setPrimaryCamera(camera);

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
