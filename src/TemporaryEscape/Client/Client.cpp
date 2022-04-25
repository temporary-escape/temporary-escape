#include "Client.hpp"
#include "../Utils/Random.hpp"
#include <fstream>

#define CMP "Client"

using namespace Engine;

Client::Client(const Config& config, Stats& stats, const std::string& address, const int port)
    : NetworkTcpClient(*this), stats(stats), sync(getWorker()) {

    const auto profilePath = config.userdataPath / Path("profile.yml");
    if (Fs::exists(profilePath)) {
        localProfile.fromYaml(profilePath);
    } else {
        localProfile.secret = randomId();
        localProfile.name = "Some Player";
        localProfile.toYaml(profilePath);
    }

    NetworkTcpClient<Client, ServerSink>::connect(address, port);

    Promise<void> promise;
    auto future = promise.future();

    MessageLogin::Request req{};
    req.secret = localProfile.secret;
    req.name = localProfile.name;

    send(req, [&](MessageLogin::Response res) {
        if (!res.error.empty()) {
            promise.reject<std::runtime_error>(res.error);
        } else {
            playerId = res.playerId;
            promise.resolve();
        }
    });

    auto t0 = std::chrono::steady_clock::now();
    while (const auto status = future.waitFor(std::chrono::milliseconds(10)) != std::future_status::ready) {
        sync.restart();
        sync.run();

        auto now = std::chrono::steady_clock::now();
        if (now - t0 > std::chrono::milliseconds(1000)) {
            NetworkTcpClient<Client, ServerSink>::stop();
            EXCEPTION("Timeout while waiting for login");
        }
    }
    future.get();
}

Client::~Client() {
    NetworkTcpClient<Client, ServerSink>::stop();
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

/*void Client::handle(MessageLogin::Response res) {
    if (!res.error.empty()) {
        loggedIn.reject<std::runtime_error>(res.error);
    } else {
        playerId = res.playerId;
        loggedIn.resolve();

        store.player.id = playerId;
        store.player.id.markChanged();
    }
}*/

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

    playerLocation = res.location;
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

/*void Client::handle(MessageFetchGalaxy::Response res) {
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
}*/

/*void Client::handle(MessageShipMovement::Response res) {
}*/
