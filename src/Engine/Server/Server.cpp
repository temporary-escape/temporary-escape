#include "Server.hpp"
#include "../Network/NetworkTcpServer.hpp"
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
#include <sol/sol.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

Server* Server::instance;

Server::Server(const Config& config, AssetsManager& assetsManager, Database& db) :
    config{config},
    assetsManager{assetsManager},
    db{db},
    generator{Generator::Options{}, assetsManager, db},
    playerSessions{config, db},
    lobby{config},
    tickFlag{true},
    worker{4},
    loadQueue{1},
    strand{worker.getService()} {

    instance = this;

    auto& dispatcher = static_cast<NetworkDispatcher&>(*this);
    HANDLE_REQUEST(MessageLoginRequest);
    HANDLE_REQUEST(MessageModManifestsRequest);
    HANDLE_REQUEST(MessagePingResponse);
    HANDLE_REQUEST(MessagePlayerSpawnRequest);
    HANDLE_REQUEST(MessageControlMovementEvent);
    HANDLE_REQUEST(MessageControlTargetEvent);

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
    eventBus = std::make_unique<EventBus>();
    lua = std::make_unique<Lua>(config, *eventBus);

    try {
        lua->importModule("base", "server.lua");
    } catch (...) {
        EXCEPTION_NESTED("Failed to import base assets module");
    }

    try {
        const auto t0 = std::chrono::high_resolution_clock::now();
        generator.generate(123456789LL);
        const auto t1 = std::chrono::high_resolution_clock::now();
        const std::chrono::duration<float> duration = std::chrono::duration_cast<std::chrono::seconds>(t1 - t0);
        logger.info("Universe has been generated in {} seconds and is ready", duration.count());
    } catch (...) {
        EXCEPTION_NESTED("Failed to generate the universe");
    }

    try {
        networkServer = std::make_unique<NetworkTcpServer>(worker.getService(), *this, config.serverPort, true);
        logger.info("TCP server started");
    } catch (...) {
        EXCEPTION_NESTED("Failed to start the server");
    }

    EventData eventData{};
    eventData["seed"] = 123456789LL;
    eventBus->enqueue("server_started", eventData);
}

void Server::cleanup() {
    logger.info("Cleanup started");

    logger.info("Clearing lobby");
    lobby.clear();

    logger.info("Clearing sessions");
    playerSessions.clear();

    logger.info("Waiting for network to stop");
    if (networkServer) {
        networkServer->stop();
    }

    logger.info("Waiting for load queue to stop");
    loadQueue.stop();

    logger.info("Waiting for workers to stop");
    worker.stop();
    networkServer.reset();

    logger.info("Clearing sectors");
    {
        std::unique_lock<std::shared_mutex> lock{sectors.mutex};
        sectors.map.clear();
    }

    logger.info("Stopping event bus");
    eventBus.reset();

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

void Server::onAcceptSuccess(const NetworkPeerPtr& peer) {
    logger.info("New connection peer: {}", peer->getAddress());
    lobby.addPeer(peer);
}

void Server::onDisconnect(const NetworkPeerPtr& peer) {
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
        auto sectorPtr = std::make_shared<Sector>(
            config, db, assetsManager, *eventBus, generator, sector.galaxyId, sector.systemId, sector.id);
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

void Server::handle(Request<MessageLoginRequest> req) {
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

void Server::handle(Request<MessagePlayerSpawnRequest> req) {
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

void Server::handle(Request<MessageModManifestsRequest> req) {
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

void Server::handle(Request<MessagePingResponse> req) {
    const auto session = playerSessions.getSession(req.peer);
    session->setLastPingTime(std::chrono::steady_clock::now());
    session->clearFlag(Session::Flags::PingSent);
}

void Server::handle(Request<MessageControlMovementEvent> req) {
    const auto session = playerSessions.getSession(req.peer);
    const auto sector = getSectorForSession(session);
    if (!sector) {
        return;
    }

    const auto data = req.get();
    sector->handle(session, data);
}

void Server::handle(Request<MessageControlTargetEvent> req) {
    const auto session = playerSessions.getSession(req.peer);
    const auto sector = getSectorForSession(session);
    if (!sector) {
        return;
    }

    const auto data = req.get();
    sector->handle(session, data);
}

void Server::bind(Lua& lua) {
    /**
     * @module engine
     */
    auto& m = lua.root();

    /**
     * @class Server
     * A class that represents server operations
     */
    auto cls = m.new_usertype<Server>("Server");
    /**
     * @function Server:move_player_to_sector
     * Moves a player to sector and starts the sector if it is not running
     * @param player_id The ID of the player to move
     * @praam sector_id The ID of the sector
     */
    cls["move_player_to_sector"] = &Server::movePlayerToSector;
    /**
     * @function Server:add_sector_type
     * Adds a new sector type, or overwrites an existing sector type, for the galaxy generator.
     * @param name Name of the sector type
     * @param type An instance of SectorType
     */
    cls["add_sector_type"] = &Server::addSectorType;
}
