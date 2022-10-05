#include "Client.hpp"
#include "../Utils/Random.hpp"
#include <fstream>

#define CMP "Client"

using namespace Engine;

Client::Client(const Config& config, Registry& registry, Stats& stats, const Path& profilePath) :
    NetworkTcpClient{*this}, registry{registry}, stats{stats}, sync{getWorker()} {

    localProfile.fromYaml(profilePath);
}

Client::~Client() {
    NetworkTcpClient<Client, ServerSink>::stop();
}

Future<void> Client::connect(const std::string& address, const int port) {
    return std::async([this, address, port]() {
        NetworkTcpClient<Client, ServerSink>::connect(address, port);

        auto promise = std::make_shared<Promise<void>>();
        fetchModInfo(promise);
        auto future = promise->future();

        if (future.waitFor(std::chrono::milliseconds(3000)) != std::future_status::ready) {
            EXCEPTION("Login timeout");
        }

        future.get();
    });
}

void Client::fetchModInfo(std::shared_ptr<Promise<void>> promise) {
    Log::i(CMP, "Fetching server mod info...");
    MessageModsInfo::Request reqModInfo{};
    send(reqModInfo, [=](MessageModsInfo::Response res) {
        const auto& ourManifests = registry.getManifests();

        for (const auto& manifest : res.manifests) {
            Log::i(CMP, "Checking for server mod: '{}' @{}", manifest.name, manifest.version);

            const auto it = std::find_if(ourManifests.begin(), ourManifests.end(),
                                         [&](const auto& m) { return m.name == manifest.name; });

            if (it == ourManifests.end()) {
                promise->reject<std::runtime_error>(fmt::format("Client is missing mod pack: '{}'", manifest.name));
                return;
            }

            const auto& found = *it;

            if (found.version != manifest.version) {
                promise->reject<std::runtime_error>(
                    fmt::format("Client has mod pack: '{}' of version: {} but server has: {}", manifest.name,
                                found.version, manifest.version));
                return;
            }
        }

        fetchLogin(promise);
    });
}

void Client::fetchLogin(std::shared_ptr<Promise<void>> promise) {
    Log::i(CMP, "Doing player login...");
    MessageLogin::Request req{};
    req.secret = localProfile.secret;
    req.name = localProfile.name;

    send(req, [=](MessageLogin::Response res) {
        if (!res.error.empty()) {
            Log::e(CMP, "Login error: {}", res.error);
            promise->reject<std::runtime_error>(res.error);
        } else {
            Log::i(CMP, "Login success");
            playerId = res.playerId;
            fetchSpawnRequest(promise);
        }
    });
}

void Client::fetchSpawnRequest(std::shared_ptr<Promise<void>> promise) {
    Log::i(CMP, "Sending spawn request...");
    MessageRequestSpawn::Request req{};

    send(req, [=](MessageRequestSpawn::Response res) {
        Log::i(CMP, "Got spawn location from the server");
        playerLocation = res.location;
        promise->resolve();
    });
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

void Client::handle(MessagePlayerLocationChanged::Response res) {
    Log::i(CMP, "Sector has changed, creating new scene");

    camera.reset();
    scene.reset();
    scene = std::make_unique<Scene>();

    camera = std::make_shared<Entity>();
    auto cmp = camera->addComponent<ComponentCamera>();
    camera->addComponent<ComponentUserInput>(*cmp);
    cmp->setProjection(80.0f);
    cmp->lookAt({3.0f, 3.0f, 3.0f}, {0.0f, 0.0f, 0.0f});
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

            // TODO:
            /*if (auto cmp = entity->findComponent<ComponentPlayer>(); cmp != nullptr && cmp->getPlayerId() == playerId)
            { camera->getComponent<ComponentCameraTurntable>()->follow(entity);
            }*/
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
