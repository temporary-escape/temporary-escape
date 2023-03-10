#include "client.hpp"
#include "../utils/random.hpp"
#include <fstream>

using namespace Engine;

static auto logger = createLogger(__FILENAME__);

#undef HANDLE_REQUEST
#define HANDLE_REQUEST(Req) addHandler([this](const PeerPtr& peer, Req req) -> void { this->handle(std::move(req)); });

Client::Client(const Config& config, Registry& registry, const PlayerLocalProfile& localProfile) :
    registry{registry}, localProfile{localProfile} {

    Network::Client::start();

    HANDLE_REQUEST(MessagePlayerLocationChanged);
    HANDLE_REQUEST(MessagePingRequest);
}

Client::~Client() {
    stop();
}

void Client::stop() {
    Network::Client::stop();
}

void Client::connect(const std::string& address, const int port) {
    logger.info("Connecting to: {} port: {}", address, port);
    Network::Client::connect(address, port);

    logger.info("Connected!");

    auto promise = std::make_shared<Promise<void>>();
    fetchModInfo(promise);
    auto future = promise->future();

    if (future.waitFor(std::chrono::milliseconds(3000)) != std::future_status::ready) {
        EXCEPTION("Login timeout");
    }

    future.get();
}

void Client::fetchModInfo(std::shared_ptr<Promise<void>> promise) {
    logger.info("Fetching server mod info...");
    MessageModsInfoRequest reqModInfo{};
    send(reqModInfo, [=](MessageModsInfoResponse res) {
        const auto& ourManifests = registry.getManifests();

        for (const auto& manifest : res.manifests) {
            logger.info("Checking for server mod: '{}' @{}", manifest.name, manifest.version);

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
    logger.info("Doing player login...");
    MessageLoginRequest req{};
    req.secret = localProfile.secret;
    req.name = localProfile.name;

    send(req, [=](MessageLoginResponse res) {
        if (!res.error.empty()) {
            logger.error("Login error: {}", res.error);
            promise->reject<std::runtime_error>(res.error);
        } else {
            logger.info("Login success");
            playerId = res.playerId;
            fetchSpawnRequest(promise);
        }
    });
}

void Client::fetchSpawnRequest(std::shared_ptr<Promise<void>> promise) {
    logger.info("Sending spawn request...");
    MessageSpawnRequest req{};

    send(req, [=](MessageSpawnResponse res) {
        logger.info("Got spawn location from the server");
        playerLocation = res.location;
        promise->resolve();

        // Continue without the promise
        fetchSystemInfo();
    });
}

void Client::fetchSystemInfo() {
    MessageFetchSystemRequest req{};
    req.systemId = playerLocation.systemId;
    req.galaxyId = playerLocation.galaxyId;

    send(req, [=](MessageFetchSystemResponse res) {
        logger.info("Got system info for player location");

        systemSeed = res.system.seed;
    });
}

void Client::update() {
    sync.poll();

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

void Client::handle(MessagePlayerLocationChanged res) {
    logger.info("Sector has changed, creating new scene");

    camera.reset();
    scene.reset();
    scene = std::make_unique<Scene>();

    auto sun = scene->createEntity();
    sun->addComponent<ComponentDirectionalLight>(Color4{1.0f, 0.9f, 0.8f, 1.0f});
    sun->addComponent<ComponentTransform>().translate(Vector3{3.0f, 1.0f, 3.0f});

    camera = scene->createEntity();
    auto& cameraTransform = camera->addComponent<ComponentTransform>();
    auto& cameraCamera = camera->addComponent<ComponentCamera>(cameraTransform);
    camera->addComponent<ComponentUserInput>(cameraCamera);
    cameraCamera.setProjection(80.0f);
    cameraCamera.lookAt({3.0f, 3.0f, 3.0f}, {0.0f, 0.0f, 0.0f});
    logger.info("Setting scene primary camera");
    scene->setPrimaryCamera(camera);

    auto entity = scene->createEntity();
    auto& entityTransform = entity->addComponent<ComponentTransform>();
    auto& debug = entity->addComponent<ComponentDebug>();
    auto& grid = entity->addComponent<ComponentGrid>(debug);
    grid.setDirty(true);

    auto block = registry.getBlocks().find("block_crew_quarters_t1");
    for (auto a = 0; a < 4; a++) {
        for (auto b = 0; b < 4; b++) {
            grid.insert(Vector3i{a, 0, b}, block, a * b, 0, VoxelShape::Type::Cube);
        }
    }
    grid.insert(Vector3i{2, 1, 2}, block, 0, 0, VoxelShape::Type::Cube);

    playerLocation = res.location;
}

void Client::handle(MessageSceneEntitiesChanged res) {
    try {
        if (!scene) {
            EXCEPTION("Client scene is not initialized");
        }
        /*for (auto& proxy : res.entities) {
            const auto entity = std::dynamic_pointer_cast<Entity>(proxy);
            if (!entity) {
                EXCEPTION("Failed to cast EntityProxy to Entity");
            }
            scene->addEntity(entity);

            // TODO:
            //if (auto cmp = entity->findComponent<ComponentPlayer>(); cmp != nullptr && cmp->getPlayerId() == playerId)
            //{ camera->getComponent<ComponentCameraTurntable>()->follow(entity);
            //}
        }*/
    } catch (...) {
        EXCEPTION_NESTED("Failed to process scene entities");
    }
}

void Client::handle(MessageSceneDeltasChanged res) {
    try {
        if (!scene) {
            EXCEPTION("Client scene is not initialized");
        }
        /*for (auto& delta : res.deltas) {
            scene->updateEntity(delta);
        }*/
    } catch (...) {
        EXCEPTION_NESTED("Failed to process scene entities");
    }
}

void Client::handle(MessagePingRequest req) {
    MessagePingResponse res;
    res.time = req.time;
    send(res);
}

void Client::onError(std::error_code ec) {
    logger.error("Server network error: {} ({})", ec.message(), ec.category().name());
}

void Client::onError(const PeerPtr& peer, std::error_code ec) {
    logger.error("Server network error: {} ({})", ec.message(), ec.category().name());
    peer->close();
}

void Client::onUnhandledException(const PeerPtr& peer, std::exception_ptr& eptr) {
    try {
        std::rethrow_exception(eptr);
    } catch (std::exception& e) {
        BACKTRACE(e, "Server network error");
    }
    peer->close();
}

void Client::postDispatch(std::function<void()> fn) {
    sync.postSafe(std::forward<decltype(fn)>(fn));
}
