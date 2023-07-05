#include "client.hpp"
#include "../scene/controllers/controller_network.hpp"
#include "../utils/random.hpp"
#include <fstream>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

#undef HANDLE_REQUEST
#define HANDLE_REQUEST(Req) addHandler([this](const PeerPtr& peer, Req req) -> void { this->handle(std::move(req)); });

Client::Client(const Config& config, AssetsManager& assetsManager, const PlayerLocalProfile& localProfile) :
    assetsManager{assetsManager}, localProfile{localProfile} {

    Network::Client::start();

    HANDLE_REQUEST(MessagePlayerLocationEvent);
    HANDLE_REQUEST(MessagePingRequest);
    addHandler(this, &Client::handleSceneSnapshot, "MessageComponentSnapshot");
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

    Future<void> future = std::async([this]() { doLogin(); });
    if (future.waitFor(std::chrono::milliseconds(3000)) != std::future_status::ready) {
        EXCEPTION("Login timeout");
    }

    try {
        future.get();
    } catch (...) {
        EXCEPTION_NESTED("Login failed");
    }
}

void Client::doLogin() {
    { // Fetch mod manifest
        logger.info("Doing manifest check...");
        MessageModManifestsRequest req{};

        auto future = send(req, useFuture<MessageModManifestsResponse>());
        auto res = future.get(std::chrono::seconds(1));
        validateManifests(res.items);

        logger.info("Manifest check success");
    }

    { // Log in
        logger.info("Doing player login...");
        MessageLoginRequest req{};
        req.secret = localProfile.secret;
        req.name = localProfile.name;

        auto future = send(req, useFuture<MessageLoginResponse>());
        auto res = future.get(std::chrono::seconds(1));

        logger.info("Login success");
        playerId = res.playerId;
    }

    { // Wait until we know where we are
        logger.info("Waiting for player location...");
        auto timeout = std::chrono::system_clock::now() + std::chrono::seconds(10);
        while (timeout > std::chrono::system_clock::now()) {
            MessagePlayerLocationRequest req{};

            auto future = send(req, useFuture<MessagePlayerLocationResponse>());
            auto res = future.get(std::chrono::seconds(1));

            if (res.location) {
                playerLocation = *res.location;
                break;
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        if (playerLocation.sectorId.empty()) {
            EXCEPTION("Timeout while waiting for spawn location");
        }
    }
}

void Client::validateManifests(const std::vector<ModManifest>& serverManifests) {
    const auto& ourManifests = assetsManager.getManifests();
    for (const auto& manifest : serverManifests) {
        logger.info("Checking for server mod: '{}' @{}", manifest.name, manifest.version);

        const auto it = std::find_if(
            ourManifests.begin(), ourManifests.end(), [&](const auto& m) { return m.name == manifest.name; });

        if (it == ourManifests.end()) {
            EXCEPTION("Client is missing mod pack: '{}'", manifest.name);
        }

        const auto& found = *it;

        if (found.version != manifest.version) {
            EXCEPTION("Client has mod pack: '{}' of version: {} but server has: {}",
                      manifest.name,
                      found.version,
                      manifest.version);
        }
    }
}

void Client::update() {
    sync.poll();
}

void Client::createScene(SectorData sector) {
    logger.info("Sector has changed, creating new scene");

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
    skybox.addComponent<ComponentSkybox>(sector.seed);

    auto planet = scene->createEntity();
    planet.addComponent<ComponentTransform>().translate(Vector3{-1.0f, 0.0f, 0.0f} * 1.0f);
    planet.getComponent<ComponentTransform>().scale(Vector3{1.0f});
    planet.addComponent<ComponentPlanet>(planetLife, 7869732137);

    planet = scene->createEntity();
    planet.addComponent<ComponentTransform>().translate(Vector3{0.7f, 0.0f, -1.5f} * 1.0f);
    planet.getComponent<ComponentTransform>().scale(Vector3{2.5f});
    planet.addComponent<ComponentPlanet>(planetSulfur, 123478);

    auto camera = scene->createEntity();
    auto& cameraTransform = camera.addComponent<ComponentTransform>();
    auto& cameraCamera = camera.addComponent<ComponentCamera>(cameraTransform);
    cameraCamera.setProjection(80.0f);
    cameraCamera.lookAt({3.0f, 3.0f, 3.0f}, {0.0f, 0.0f, 0.0f});
    logger.info("Setting scene primary camera");
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

void Client::handle(MessagePlayerLocationEvent res) {
    logger.info("Player location has changed");
    playerLocation = res.location;

    MessageFetchSectorRequest req{};
    req.galaxyId = res.location.galaxyId;
    req.systemId = res.location.systemId;
    req.sectorId = res.location.sectorId;

    sync.postSafe([this]() {
        scene.reset();
        scene = std::make_unique<Scene>();
    });

    send(req, [this](MessageFetchSectorResponse res) {
        if (!res.error.empty()) {
            EXCEPTION("Unable to fetch sector at player's location");
        }
        createScene(std::move(res.sector));
    });
}

void Client::handleSceneSnapshot(const PeerPtr& peer, Network::RawMessage message) {
    (void)peer;

    sync.postSafe([this, msg = std::move(message)]() {
        logger.debug("Applying scene snapshot");
        scene->getController<ControllerNetwork>().receiveSnapshot(msg.get());
    });
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
