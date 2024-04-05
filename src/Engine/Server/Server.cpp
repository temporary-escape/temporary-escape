#include "Server.hpp"
#include "../Database/SaveInfo.hpp"
#include "../Network/NetworkUdpServer.hpp"
#include "../Utils/Random.hpp"
#include "Lua.hpp"
#include "Services/ServiceFactions.hpp"
#include "Services/ServiceGalaxy.hpp"
#include "Services/ServicePlanets.hpp"
#include "Services/ServicePlayers.hpp"
#include "Services/ServiceRegions.hpp"
#include "Services/ServiceSectors.hpp"
#include "Services/ServiceSystems.hpp"
#include <memory>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

Server* Server::instance;

static DatabaseRocksDB::Options getDatabaseOptions(const Config& config) {
    DatabaseRocksDB::Options options{};
    options.cacheSizeMb = config.server.dbCacheSize;
    options.debugLogging = config.server.dbDebug;
    options.compression = config.server.dbCompression;
    return options;
}

Server::Server(const Config& config, AssetsManager& assetsManager, const Options& options) :
    config{config},
    assetsManager{assetsManager},
    options{options},
    db{options.savePath, getDatabaseOptions(config)},
    playerSessions{config, db},
    lobby{config},
    tickFlag{true},
    worker{4},
    loadQueue{1},
    strand{worker.getService()} {

    instance = this;

    auto& dispatcher = static_cast<NetworkDispatcher2&>(*this);
    HANDLE_REQUEST2(MessageLoginRequest);
    HANDLE_REQUEST2(MessageModManifestsRequest);
    HANDLE_REQUEST2(MessagePingResponse);
    HANDLE_REQUEST2(MessagePlayerSpawnRequest);
    HANDLE_REQUEST2(MessageActionApproach);
    HANDLE_REQUEST2(MessageActionOrbit);
    HANDLE_REQUEST2(MessageActionKeepDistance);
    HANDLE_REQUEST2(MessageActionStopMovement);
    HANDLE_REQUEST2(MessageActionGoDirection);
    HANDLE_REQUEST2(MessageActionWarpTo);
    HANDLE_REQUEST2(MessageControlTargetEvent);

    addService<ServicePlayers>();
    addService<ServiceGalaxy>();
    addService<ServiceFactions>();
    addService<ServiceRegions>();
    addService<ServiceSystems>();
    addService<ServiceSectors>();
    addService<ServicePlanets>();

    try {
        load();
    } catch (...) {
        cleanup();
        EXCEPTION_NESTED("Failed to load server");
    }

    startTick();
}

EventBus& Server::getEventBus() const {
    if (!eventBus) {
        EXCEPTION("EventBus is not initialized");
    }
    return *eventBus;
}

void Server::load() {
    auto seed = db.find<MetaData>("seed");
    if (!seed) {
        if (options.seed == 0) {
            EXCEPTION("Unable to start a new save file with zero seed");
        }

        seed = MetaData{static_cast<int64_t>(options.seed)};
        db.put("seed", *seed);
    }

    generator = std::make_unique<Generator>(Generator::Options{}, assetsManager, db);
    eventBus = std::make_unique<EventBus>();
    lua = std::make_unique<Lua>(config, *eventBus);

    try {
        lua->importModule("base", "server.lua");
    } catch (...) {
        EXCEPTION_NESTED("Failed to import base assets module");
    }

    try {
        const auto t0 = std::chrono::high_resolution_clock::now();
        generator->generate(std::get<int64_t>(seed->value));
        const auto t1 = std::chrono::high_resolution_clock::now();
        const std::chrono::duration<float> duration = std::chrono::duration_cast<std::chrono::seconds>(t1 - t0);
        logger.info("Universe has been generated in {} seconds and is ready", duration.count());
    } catch (...) {
        EXCEPTION_NESTED("Failed to generate the universe");
    }

    try {
        network = std::make_shared<NetworkUdpServer>(config, worker.getService(), *this);
        network->start();
        port = network->getEndpoint().port();
        logger.info("TCP server started");
    } catch (...) {
        EXCEPTION_NESTED("Failed to start the server");
    }

    try {
        updateSaveInfo();
    } catch (std::exception& e) {
        EXCEPTION_NESTED("Failed to create save info");
    }

    EventData eventData{};
    eventData["seed"] = std::get<int64_t>(seed->value);
    eventBus->enqueue("server_started", eventData);
}

void Server::updateSaveInfo() {
    SaveInfo saveInfo{};
    saveInfo.version = GAME_VERSION;
    saveInfo.timestamp = std::chrono::system_clock::now();
    Xml::toFile(options.savePath / "info.xml", saveInfo);
}

void Server::cleanup() {
    try {
        updateSaveInfo();
    } catch (std::exception& e) {
        logger.error("Failed to update save file info error: {}", e.what());
    }

    logger.info("Cleanup started");

    logger.info("Clearing lobby");
    lobby.clear();

    logger.info("Clearing sessions");
    playerSessions.clear();

    logger.info("Waiting for network to stop");
    if (network) {
        network->stop();
    }

    logger.info("Waiting for load queue to stop");
    loadQueue.stop();

    logger.info("Waiting for workers to stop");
    worker.stop();
    network.reset();

    logger.info("Clearing sectors");
    {
        std::unique_lock<std::shared_mutex> lock{sectors.mutex};
        sectors.map.clear();
    }

    logger.info("Stopping event bus");
    eventBus.reset();

    logger.info("Stopping generator");
    generator.reset();

    logger.info("Stopping lua");
    lua.reset();

    logger.info("Stop done");
}

Server::~Server() {
    logger.info("Stopping server thread");

    tickFlag.store(false);
    if (tickThread.joinable()) {
        tickThread.join();
    }
}

void Server::startTick() {
    tickThread = std::thread([this]() {
        try {
            tick();
        } catch (std::exception& e) {
            BACKTRACE(e, "Server tick thread error");
        }
        cleanup();
    });
}

void Server::pollEvents() {
    try {
        eventBus->poll();
    } catch (std::exception& e) {
        EXCEPTION_NESTED("Error while polling event bus");
    }
}

void Server::updateSectors() {
    std::shared_lock<std::shared_mutex> lock{sectors.mutex};
    for (auto& [compoundId, sector] : sectors.map) {
        // Skip sectors that are not yet ready
        if (!sector->isLoaded()) {
            continue;
        }

        try {
            sector->update();
        } catch (std::exception& e) {
            EXCEPTION_NESTED("Failed to update sector: '{}'", compoundId);
        }
    }
}

void Server::tick() {
    logger.info("Starting tick");

    while (tickFlag.load()) {
        const auto start = std::chrono::high_resolution_clock::now();

        // playerSessions.updateSessionsPing();
        pollEvents();
        updateSectors();

        const auto now = std::chrono::high_resolution_clock::now();
        const auto test = std::chrono::duration_cast<std::chrono::microseconds>(now - start);
        if (test < config.tickLengthUs) {
            std::this_thread::sleep_for(config.tickLengthUs - test);
        }

        perf.tickTime.update(std::chrono::duration_cast<std::chrono::nanoseconds>(now - start));
    }

    logger.info("Stopped tick");
}

void Server::onAcceptSuccess(const NetworkStreamPtr& peer) {
    logger.info("New connection peer: {}", peer->getAddress());
    lobby.addPeer(peer);
}

void Server::onDisconnect(const NetworkStreamPtr& peer) {
    logger.info("Disconnected connection peer: {}", peer->getAddress());
    lobby.removePeer(peer);
    playerSessions.removeSession(peer);
}

void Server::movePlayerToSector(const std::string& playerId, const std::string& sectorId) {
    auto session = getPlayerSession(playerId);
    if (!session) {
        EXCEPTION("Can not move player id: '{}' to sector: '{}', player is not logged in", playerId, sectorId);
    }

    startSector(sectorId);
    addPlayerToSector(session, sectorId);
}

SessionPtr Server::getPlayerSession(const std::string& playerId) {
    return playerSessions.getSession(playerId);
}

SectorPtr Server::startSector(const std::string& sectorId) {
    logger.info("Starting sector: '{}'", sectorId);

    auto sectorOpt = db.getByIndex<&SectorData::id>(sectorId);
    if (sectorOpt.empty()) {
        EXCEPTION("Can not start sector: '{}' not found", sectorId);
    }
    auto& sector = sectorOpt.front();

    std::unique_lock<std::shared_mutex> lock{sectors.mutex};

    auto it = sectors.map.find(sector.id);
    if (it != sectors.map.end()) {
        logger.info("Starting sector: '{}' skipped, already started", sectorId);
        return it->second;
    }

    try {
        logger.info("Creating sector: '{}'", sectorId);
        auto sectorPtr =
            std::make_shared<Sector>(config, db, assetsManager, *eventBus, sector.galaxyId, sector.systemId, sector.id);
        sectors.map.insert(std::make_pair(sector.id, sectorPtr));

        // Load the sector in a separate thread
        loadQueue.post([sectorPtr]() {
            try {
                sectorPtr->load();
            } catch (std::exception& e) {
                BACKTRACE(e, "Failed to load sector: '{}'", sectorPtr->getSectorId());
            }
        });

        return sectorPtr;
    } catch (...) {
        EXCEPTION_NESTED("Failed to start sector: '{}'", sector.id);
    }
}

void Server::addPlayerToSector(const SessionPtr& session, const std::string& sectorId) {
    logger.info("Adding player: {} to sector: {}", session->getPlayerId(), sectorId);

    std::shared_lock<std::shared_mutex> lock{sectors.mutex};
    const auto it = sectors.map.find(sectorId);
    if (it == sectors.map.end()) {
        EXCEPTION("Can not add player to invalid sector id: {} player id: {}", sectorId, session->getPlayerId());
    }

    it->second->addPlayer(session);
}

void Server::disconnectPlayer(const std::string& playerId) {
    logger.info("Disconnecting player: {}", playerId);

    auto session = playerSessions.getSession(playerId);
    if (session) {
        playerSessions.removeSession(session->getStream());
        session->close();
    }
}

SectorPtr Server::getSectorForSession(const SessionPtr& session) {
    // Find where the player is located
    const auto location = playerSessions.getLocation(session);
    if (!location) {
        logger.warn("Player {} has bad location", session->getPlayerId());
        return nullptr;
    }

    // Forward the message to the sector
    std::unique_lock<std::shared_mutex> lock{sectors.mutex};
    const auto found = sectors.map.find(*location);
    if (found == sectors.map.end()) {
        // Maybe the sector no longer exists?
        logger.warn("Player {} has a non-existing location", session->getPlayerId());
        return nullptr;
    }

    return found->second;
}

template <typename T> void Server::forwardMessageToSector(const Request2<T>& req) {
    const auto session = playerSessions.getSession(req.peer);
    const auto sector = getSectorForSession(session);
    if (!sector) {
        return;
    }

    sector->handle(session, req.get());
}

void Server::handle(Request2<MessageLoginRequest> req) {
    logger.info("New login peer: {}", req.peer->getAddress());

    auto data = req.get();

    // Check for server password
    if (!config.serverPassword.empty() && config.serverPassword != data.password) {
        req.respondError("Bad server password");
        lobby.disconnectPeer(req.peer);
        return;
    }

    auto& servicePlayers = getService<ServicePlayers>();

    // Check if the player is already logged in
    const auto playerIdFound = servicePlayers.secretToId(data.secret);
    if (playerIdFound) {
        if (playerSessions.isLoggedIn(*playerIdFound)) {
            req.respondError("Already logged in");
            lobby.disconnectPeer(req.peer);
            return;
        }
    }

    // Remove peer from lobby
    lobby.removePeer(req.peer);

    // Login
    logger.info("Logging in player name: '{}'", data.name);

    PlayerData player;
    try {
        player = servicePlayers.login(data.secret, data.name);
    } catch (...) {
        req.respondError("Failed logging in");
        lobby.disconnectPeer(req.peer);
        EXCEPTION_NESTED("Failed to log in player: '{}'", data.name);
    }

    logger.info("Logged in player: {} name: '{}'", player.id, data.name);

    auto session = playerSessions.createSession(req.peer, player.id);

    MessageLoginResponse res{};
    res.playerId = player.id;
    req.respond(res);

    // Publish an event
    EventData event{};
    event["player_id"] = player.id;
    event["player_name"] = player.name;
    eventBus->enqueue("player_logged_in", event);
}

void Server::handle(Request2<MessagePlayerSpawnRequest> req) {
    auto session = playerSessions.getSession(req.peer);

    if (const auto location = playerSessions.getLocation(session); location.has_value()) {
        // Player already has location
        logger.warn("Player {} has requested spawn but the player is already spawned in sector: {}",
                    session->getPlayerId(),
                    *location);
        return;
    }

    logger.info("Player {} has requested spawn", session->getPlayerId());

    auto& servicePlayers = getService<ServicePlayers>();

    // Find the spawn location for the player
    const auto location = servicePlayers.getSpawnLocation(session->getPlayerId());

    // Remember the player location
    playerSessions.setLocation(session, location.sectorId);

    // Start the sector and add the player to it
    startSector(location.sectorId);
    addPlayerToSector(session, location.sectorId);
}

void Server::handle(Request2<MessageModManifestsRequest> req) {
    logger.info("Peer {} has requested mod manifest", req.peer->getAddress());

    MessageModManifestsResponse res{};

    const auto& manifests = assetsManager.getManifests();
    res.items.reserve(manifests.size());
    for (const auto& manifest : manifests) {
        res.items.push_back(manifest);
    }
    res.page.hasNext = false;

    req.respond(res);
}

void Server::handle(Request2<MessagePingResponse> req) {
    const auto session = playerSessions.getSession(req.peer);
    session->setLastPingTime(std::chrono::steady_clock::now());
    session->clearFlag(Session::Flags::PingSent);
}

void Server::handle(Request2<MessageActionApproach> req) {
    forwardMessageToSector(req);
}

void Server::handle(Request2<MessageActionOrbit> req) {
    forwardMessageToSector(req);
}

void Server::handle(Request2<MessageActionKeepDistance> req) {
    forwardMessageToSector(req);
}

void Server::handle(Request2<MessageActionStopMovement> req) {
    forwardMessageToSector(req);
}

void Server::handle(Request2<MessageActionGoDirection> req) {
    forwardMessageToSector(req);
}

void Server::handle(Request2<MessageActionWarpTo> req) {
    forwardMessageToSector(req);
}

void Server::handle(Request2<MessageControlTargetEvent> req) {
    const auto session = playerSessions.getSession(req.peer);
    const auto sector = getSectorForSession(session);
    if (!sector) {
        return;
    }

    const auto data = req.get();
    sector->handle(session, data);
}
