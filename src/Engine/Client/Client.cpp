#include "Client.hpp"
#include "../Scene/Controllers/ControllerNetwork.hpp"
#include "../Scene/Controllers/ControllerTurret.hpp"
#include "../Server/MatchmakerClient.hpp"
#include "../Utils/Random.hpp"
#include <fstream>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

Client::Client(const Config& config, AssetsManager& assetsManager, const PlayerLocalProfile& localProfile,
               VoxelShapeCache* voxelShapeCache) :
    config{config},
    assetsManager{assetsManager},
    localProfile{localProfile},
    voxelShapeCache{voxelShapeCache},
    worker{1} {

    auto& dispatcher = static_cast<NetworkDispatcher2&>(*this);
    HANDLE_REQUEST2(MessagePlayerLocationEvent);
    HANDLE_REQUEST2(MessagePingRequest);
    HANDLE_REQUEST2(MessageModManifestsResponse);
    HANDLE_REQUEST2(MessageLoginResponse);
    HANDLE_REQUEST2(MessageFetchGalaxyResponse);
    HANDLE_REQUEST2(MessageFetchRegionsResponse);
    HANDLE_REQUEST2(MessageFetchFactionsResponse);
    HANDLE_REQUEST2(MessageFetchSystemsResponse);
    HANDLE_REQUEST2(MessageSceneUpdateEvent);
    HANDLE_REQUEST2(MessageFetchPlanetsResponse);
    HANDLE_REQUEST2(MessageFetchSectorsResponse);
    // HANDLE_REQUEST(MessageSceneBulletsEvent);
    HANDLE_REQUEST2(MessagePlayerControlEvent);
    // addHandler(this, &Client::handleSceneSnapshot, "MessageComponentSnapshot");

    network = std::make_shared<NetworkUdpClient>(config, worker.getService(), dispatcher);
    network->start();
}

Client::Client(const Config& config, AssetsManager& assetsManager, const PlayerLocalProfile& localProfile,
               VoxelShapeCache* voxelShapeCache, const std::string& address, int port) :
    Client{config, assetsManager, localProfile, voxelShapeCache} {

    connectToAddress(address, port);
}

Client::Client(const Config& config, AssetsManager& assetsManager, const PlayerLocalProfile& localProfile,
               VoxelShapeCache* voxelShapeCache, MatchmakerClient& matchmakerClient, const std::string& serverId) :
    Client{config, assetsManager, localProfile, voxelShapeCache} {

    connectToServer(matchmakerClient, serverId);
}

Client::~Client() {
    logger.info("Stopping client");
    network->stop();
    worker.stop();
    network.reset();
}

void Client::connectToServer(MatchmakerClient& matchmakerClient, const std::string& serverId) {
    auto promise = std::make_shared<Promise<void>>();

    network->getStunClient().send([this, promise, m = &matchmakerClient, serverId](
                                      const NetworkStunClient::Result& stun) {
        m->apiServersConnect(serverId, stun.endpoint, [this, promise](MatchmakerClient::ServerConnectResponse resp) {
            if (!resp.error.empty()) {
                promise->reject<std::runtime_error>(resp.error);
            } else if (resp.status != 200) {
                promise->reject<std::runtime_error>(fmt::format("Server responded with error code: {}", resp.status));
            } else {
                connectToAddress(resp.data.address, resp.data.port);
                promise->resolve();
            }
        });
    });

    auto future = promise->future();
    if (future.waitFor(std::chrono::seconds{5}) != std::future_status::ready) {
        EXCEPTION("Timeout waiting for the server to respond");
    }

    future.get();
}

void Client::connectToAddress(const std::string& address, int port) {
    try {
        network->connect(address, port);

        // Start login sequence by sending manifest request
        MessageModManifestsRequest req{};
        network->send(req);
    } catch (...) {
        network->stop();
        EXCEPTION_NESTED("Failed to connect to the server");
    }

    try {
        auto future = promiseLogin.future();
        future.get(std::chrono::seconds{2});
    } catch (...) {
        EXCEPTION_NESTED("Failed to login");
    }
}

void Client::disconnect() {
    logger.info("Disconnecting client");
    if (network) {
        network->stop();
    }
}

void Client::update(const float deltaTime) {
    sync.poll();

    if (scene) {
        scene->interpolate(deltaTime);
    }

    // Update player camera location
    if (scene && cache.player.entity != NullEntity) {
        const auto* transform = scene->tryGetComponent<ComponentTransform>(cache.player.entity);
        if (transform) {
            cache.player.position = transform->getPosition();

            if (const auto* camera = scene->getPrimaryCamera(); camera) {
                auto* cameraOrbital = scene->tryGetComponent<ComponentCameraOrbital>(camera->getEntity());
                if (cameraOrbital) {
                    cameraOrbital->setTarget(transform->getInterpolatedPosition());
                }
            }

            const auto* shipControl = scene->tryGetComponent<ComponentShipControl>(cache.player.entity);
            if (shipControl) {
                cache.player.approaching = scene->getLocalId(shipControl->getApproachEntity());
                cache.player.autopilotAction = shipControl->getAction();
                cache.player.forwardVelocity = shipControl->getForwardVelocity();
                cache.player.forwardVelocityMax = shipControl->getForwardVelocityMax();
                cache.player.targetDistance = shipControl->getTargetDistance();
                cache.player.keepAtDistance = shipControl->getKeepAtDistance();
                /*cache.player.orbitRadius = shipControl->getOrbitRadius();
                cache.player.keepAtDistance = shipControl->getApproachMinDistance();
                cache.player.forwardVelocity = shipControl->getForwardVelocity();
                cache.player.forwardVelocityMax = shipControl->getForwardVelocityMax();
                cache.player.approachDistance = shipControl->getApproachDistance();*/
            }

            auto* shipIcon = scene->tryGetComponent<ComponentIcon>(cache.player.entity);
            if (shipIcon && shipIcon->isSelectable()) {
                shipIcon->setColor(Color4{0.0f});
                shipIcon->setSelectable(false);
                scene->setDirty(*shipIcon);
            }
        }
    }

    if (scene) {
        scene->update(deltaTime);
    }
}

void Client::startCacheSync() {
    MessageFetchGalaxyRequest msg{};
    network->send(msg);
}

void Client::createScene(const SectorData& sector) {
    logger.info("Creating new scene");

    scene.reset();
    scene = std::make_unique<Scene>(config, voxelShapeCache);

    const auto starTexture = assetsManager.getTextures().find("space_sun_flare");
    const auto starTextureLow = assetsManager.getTextures().find("star_spectrum_low");
    const auto starTextureHigh = assetsManager.getTextures().find("star_spectrum_high");

    const auto planetLife = assetsManager.getPlanetTypes().find("planet_life");
    const auto planetAlien = assetsManager.getPlanetTypes().find("planet_alien");
    const auto planetDesert = assetsManager.getPlanetTypes().find("planet_desert");
    const auto planetSulfur = assetsManager.getPlanetTypes().find("planet_sulfur");

    auto sun = scene->createEntity();
    sun.addComponent<ComponentDirectionalLight>(Color4{1.0f, 0.9f, 0.8f, 1.0f});
    sun.addComponent<ComponentTransform>().translate(Vector3{3.0f, 1.0f, 3.0f} * 100.0f);
    sun.addComponent<ComponentStarFlare>(starTexture, starTextureLow, starTextureHigh);

    auto skybox = scene->createEntity();
    skybox.addComponent<ComponentSkybox>(sector.seed).setStars(true);

    auto dust = scene->createEntity();
    dust.addComponent<ComponentSpaceDust>();

    auto planet = scene->createEntity();
    planet.addComponent<ComponentTransform>().translate(Vector3{-1.0f, 0.0f, 0.0f} * 1.0f);
    planet.getComponent<ComponentTransform>().scale(Vector3{1.0f});
    planet.addComponent<ComponentPlanet>(planetLife, 7869732137).setHighRes(true);

    planet = scene->createEntity();
    planet.addComponent<ComponentTransform>().translate(Vector3{0.7f, 0.0f, -1.5f} * 1.0f);
    planet.getComponent<ComponentTransform>().scale(Vector3{2.5f});
    planet.addComponent<ComponentPlanet>(planetSulfur, 123478).setHighRes(true);

    auto camera = scene->createEntity();
    auto& cameraTransform = camera.addComponent<ComponentTransform>();
    auto& cameraCamera = camera.addComponent<ComponentCamera>(cameraTransform);
    auto& cameraOrbital = camera.addComponent<ComponentCameraOrbital>(cameraCamera);
    cameraCamera.setProjection(80.0f);
    cameraOrbital.setDistance(10.0f);
    cameraOrbital.setRotation({-45.0f, 45.0f});
    scene->setPrimaryCamera(camera);

    /*auto entity = scene->createEntity();
    auto& entityTransform = entity.addComponent<ComponentTransform>();
    auto& debug = entity.addComponent<ComponentDebug>();
    auto& grid = entity.addComponent<ComponentGrid>(debug);
    grid.setDirty(true);

    entity = scene->createEntity();
    entity.addComponent<ComponentTransform>().move({0.0f, 5.0f, 0.0f});
    entity.addComponent<ComponentModel>(assetsManager.getModels().find("model_asteroid_01_h"));

    auto block = assetsManager.getBlocks().find("block_crew_quarters_t1");
    for (auto a = 0; a < 4; a++) {
        for (auto b = 0; b < 4; b++) {
            grid.insert(Vector3i{a, 0, b}, block, a * b, 1, VoxelShape::Type::Cube);
        }
    }
    grid.insert(Vector3i{2, 1, 2}, block, 0, 1, VoxelShape::Type::Cube);*/
}

void Client::handle(Request2<MessagePlayerLocationEvent> req) {
    auto data = req.get();

    logger.info("Player location has changed");

    sync.postSafe([this, data]() {
        cache.location = data.location;
        cache.sector = data.sector;
        cache.system = data.system;

        createScene(data.sector);

        // Fetch planets for the system we are currently in
        MessageFetchPlanetsRequest msg{};
        msg.galaxyId = cache.galaxy.id;
        msg.systemId = cache.system->id;
        network->send(msg);
    });
}

/*void Client::handleSceneSnapshot(const PeerPtr& peer, Network::RawMessage message) {
    (void)peer;

    sync.postSafe([this, msg = std::move(message)]() {
        // logger.debug("Applying scene snapshot");
        scene->getController<ControllerNetwork>().receiveUpdate(msg.get());
    });
}*/

void Client::handle(Request2<MessagePingRequest> req) {
    auto data = req.get();
    MessagePingResponse res;
    res.time = data.time;
    req.respond(res);
}

void Client::handle(Request2<MessageModManifestsResponse> req) {
    if (req.isError()) {
        promiseLogin.reject<std::runtime_error>(req.getError());
        return;
    }

    auto data = req.get();

    const auto& ourManifests = assetsManager.getManifests();
    for (const auto& manifest : data.items) {
        logger.info("Checking for server mod: '{}' @{}", manifest.name, manifest.version);

        const auto it = std::find_if(
            ourManifests.begin(), ourManifests.end(), [&](const auto& m) { return m.name == manifest.name; });

        if (it == ourManifests.end()) {
            promiseLogin.reject<std::runtime_error>(fmt::format("Missing mod pack: {}", manifest.name));
            return;
        }

        const auto& found = *it;

        if (found.version != manifest.version) {
            promiseLogin.reject<std::runtime_error>(
                fmt::format("Missing mod pack: {} expected version: {} but got version: {}",
                            manifest.name,
                            found.version,
                            manifest.version));
            return;
        }
    }

    MessageLoginRequest msg{};
    msg.secret = localProfile.secret;
    msg.name = localProfile.name;
    network->send(msg);
}

void Client::handle(Request2<MessageLoginResponse> req) {
    if (req.isError()) {
        promiseLogin.reject<std::runtime_error>(req.getError());
        return;
    }

    auto data = req.get();
    cache.playerId = data.playerId;

    // Player login sequence is complete
    promiseLogin.resolve();

    // Start the sync sequence
    startCacheSync();
}

void Client::handle(Request2<MessageFetchGalaxyResponse> req) {
    auto data = req.get();

    logger.debug("Received galaxy data");

    cache.galaxy.name = data.name;
    cache.galaxy.id = data.galaxyId;

    // Fetch galaxy regions
    MessageFetchRegionsRequest msg{};
    msg.galaxyId = cache.galaxy.id;
    network->send(msg);
}

void Client::handle(Request2<MessageFetchRegionsResponse> req) {
    auto data = req.get();

    logger.debug("Received region data count: {}", data.items.size());

    for (auto& item : data.items) {
        cache.galaxy.regions.emplace(item.id, std::move(item));
    }

    if (data.page.hasNext && !data.page.token.empty()) {
        // Continue fetching the next page
        MessageFetchRegionsRequest msg{};
        msg.galaxyId = cache.galaxy.id;
        msg.token = data.page.token;
        network->send(msg);
    } else {
        // Fetch galaxy factions
        MessageFetchFactionsRequest msg{};
        msg.galaxyId = cache.galaxy.id;
        network->send(msg);
    }
}

void Client::handle(Request2<MessageFetchFactionsResponse> req) {
    auto data = req.get();

    logger.debug("Received faction data count: {}", data.items.size());

    for (auto& item : data.items) {
        cache.galaxy.factions.emplace(item.id, std::move(item));
    }

    if (data.page.hasNext && !data.page.token.empty()) {
        // Continue fetching the next page
        MessageFetchFactionsRequest msg{};
        msg.galaxyId = cache.galaxy.id;
        msg.token = data.page.token;
        network->send(msg);
    } else {
        // Fetch galaxy systems
        MessageFetchSystemsRequest msg{};
        msg.galaxyId = cache.galaxy.id;
        network->send(msg);
    }
}

void Client::handle(Request2<MessageFetchSystemsResponse> req) {
    auto data = req.get();

    logger.debug("Received system data count: {}", data.items.size());

    for (auto& item : data.items) {
        cache.galaxy.systems.emplace(item.id, std::move(item));
    }

    if (data.page.hasNext && !data.page.token.empty()) {
        // Continue fetching the next page
        MessageFetchSystemsRequest msg{};
        msg.galaxyId = cache.galaxy.id;
        msg.token = data.page.token;
        network->send(msg);
    } else {
        // Construct an ordered list of systems
        for (auto& [_, system] : cache.galaxy.systems) {
            cache.galaxy.systemsOrdered.push_back(&system);
        }

        // Mark that the cache has been synced
        flagCacheSync.store(true);

        // Request spawn location
        MessagePlayerSpawnRequest msg{};
        network->send(msg);
    }
}

void Client::handle(Request2<MessageFetchPlanetsResponse> req) {
    sync.postSafe([this, r = std::move(req)]() {
        auto data = r.get();

        logger.debug("Received planets data count: {}", data.items.size());

        cache.planets.insert(cache.planets.end(), data.items.begin(), data.items.end());

        if (data.page.hasNext && !data.page.token.empty()) {
            // Continue fetching the next page
            MessageFetchPlanetsRequest msg{};
            msg.galaxyId = cache.galaxy.id;
            msg.systemId = cache.system->id;
            msg.token = data.page.token;
            network->send(msg);
        } else {
            MessageFetchSectorsRequest msg{};
            msg.galaxyId = cache.galaxy.id;
            msg.systemId = cache.system->id;
            network->send(msg);
        }
    });
}

void Client::handle(Request2<MessageFetchSectorsResponse> req) {
    sync.postSafe([this, r = std::move(req)]() {
        auto data = r.get();

        logger.debug("Received sectors data count: {}", data.items.size());

        cache.sectors.insert(cache.sectors.end(), data.items.begin(), data.items.end());

        if (data.page.hasNext && !data.page.token.empty()) {
            // Continue fetching the next page
            MessageFetchSectorsRequest msg{};
            msg.galaxyId = cache.galaxy.id;
            msg.systemId = cache.system->id;
            msg.token = data.page.token;
            network->send(msg);
        }
    });
}

void Client::handle(Request2<MessageSceneUpdateEvent> req) {
    // logger.debug("MessageSceneUpdateEvent received");
    sync.postSafe([=, r = std::move(req)]() {
        // Update, create, or delete entities in a scene
        scene->getController<ControllerNetwork>().receiveUpdate(r.object());
    });
}

/*void Client::handle(Request<MessageSceneBulletsEvent> req) {
    sync.postSafe([=]() {
        // Update, create, or delete entities in a scene
        scene->getController<ControllerTurret>().receiveBullets(req.object());
    });
}*/

void Client::handle(Request2<MessagePlayerControlEvent> req) {
    sync.postSafe([=]() {
        const auto data = req.get();
        logger.info("Switching player control to entity: {}", data.entityId);
        const auto entity = scene->getController<ControllerNetwork>().getRemoteToLocalEntity(data.entityId);
        if (entity) {
            cache.player.entity = entity->getHandle();

            const auto transform = entity->tryGetComponent<ComponentTransform>();
            const auto camera = scene->getPrimaryCamera();

            if (!camera) {
                EXCEPTION("No camera in scene");
            }
            if (!transform) {
                EXCEPTION("Entity has no transform");
            }

            auto cameraEntity = scene->fromHandle(camera->getEntity());
            cameraEntity.getComponent<ComponentCameraOrbital>().setTarget(transform->getAbsolutePosition());
        } else {
            EXCEPTION("No local entity: {}", data.entityId);
        }
    });
}
