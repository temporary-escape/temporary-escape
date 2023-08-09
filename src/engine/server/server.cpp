#include "server.hpp"
#include "../network/network_tcp_server.hpp"
#include "../utils/random.hpp"
#include "lua.hpp"
#include "services/service_galaxy.hpp"
#include "services/service_players.hpp"
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
    loadQueue{1} {

    instance = this;

    HANDLE_REQUEST(MessageLoginRequest);
    HANDLE_REQUEST(MessageModManifestsRequest);
    HANDLE_REQUEST(MessagePlayerLocationRequest);
    HANDLE_REQUEST(MessagePingResponse);

    addService<ServicePlayers>();
    addService<ServiceGalaxy>();

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
    networkServer->stop();

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
        const auto start = std::chrono::steady_clock::now();

        // playerSessions.updateSessionsPing();
        pollEvents();
        updateSectors();

        const auto now = std::chrono::steady_clock::now();
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
        EXCEPTION("Can not move player: '{}' to sector: '{}', error: player is not logged in", playerId, sectorId);
    }

    addPlayerToSector(session, sectorId);
}

SessionPtr Server::getPlayerSession(const std::string& playerId) {
    return playerSessions.getSession(playerId);
}

SectorPtr Server::startSector(const std::string& galaxyId, const std::string& systemId, const std::string& sectorId) {
    logger.info("Starting sector: '{}/{}/{}'", galaxyId, systemId, sectorId);

    auto sectorOpt = db.find<SectorData>(fmt::format("{}/{}/{}", galaxyId, systemId, sectorId));
    if (!sectorOpt) {
        EXCEPTION_NESTED("Can not start sector: '{}/{}/{}' not found", galaxyId, systemId, sectorId);
    }
    auto& sector = sectorOpt.value();

    std::unique_lock<std::shared_mutex> lock{sectors.mutex};

    auto it = sectors.map.find(sector.id);
    if (it != sectors.map.end()) {
        logger.info("Starting sector: '{}/{}/{}' skipped, already started", galaxyId, systemId, sectorId);
        return it->second;
    }

    try {
        logger.info("Creating sector: '{}/{}/{}'", galaxyId, systemId, sectorId);
        auto sectorPtr =
            std::make_shared<Sector>(config, db, assetsManager, *eventBus, sector.galaxyId, sector.systemId, sector.id);
        sectors.map.insert(std::make_pair(sector.id, sectorPtr));

        // Load the sector in a separate thread
        loadQueue.post([this, sectorPtr]() {
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

    playerSessions.createSession(req.peer, player.id);

    MessageLoginResponse res{};
    res.playerId = player.id;
    req.respond(res);

    // Publish an event
    /*EventData event{};
    event["player_id"] = player.id;
    event["player_name"] = player.name;
    eventBus->enqueue("player_logged_in", event);*/
}

/*void Server::handle(const PeerPtr& peer, MessageSpawnRequest req, MessageSpawnResponse& res) {
    (void)req;

    auto session = playerSessions.getSession(peer);
    try {
        const auto location = playerSessions.findStartingLocation(session->getPlayerId());
        res.location = location;

        startSector(location.galaxyId, location.systemId, location.sectorId);
        addPlayerToSector(session, location.sectorId);
    } catch (...) {
        EXCEPTION_NESTED("Failed to find starting location for player: '{}'", session->getPlayerId());
    }
}*/

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

void Server::handle(Request<MessagePlayerLocationRequest> req) {
    MessagePlayerLocationResponse res{};

    const auto session = playerSessions.getSession(req.peer);
    const auto location = db.find<PlayerLocationData>(session->getPlayerId());
    if (location) {
        res.location = *location;
    } else {
        logger.warn("Player location requested for: '{}' but player has no location", session->getPlayerId());
    }

    req.respond(res);
}

void Server::handle(Request<MessagePingResponse> req) {
    auto session = playerSessions.getSession(req.peer);
    session->setLastPingTime(std::chrono::steady_clock::now());
    session->clearFlag(Session::Flags::PingSent);
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
     * @function Server:set_generator
     * Sets the function to be called when generating the universe.
     * @param fn function A callback function that accepts a seed as a number
     */
    /*cls["set_generator"] = [](Server& self, sol::function fn) {
        self.setGenerator([fn](uint64_t seed) {
            sol::protected_function_result result = fn(seed);
            if (!result.valid()) {
                sol::error err = result;
                EXCEPTION("{}", err.what());
            }
        });
    };*/
    /**
     * @function Server:start_sector
     * Starts a sector. The function won't do anything if the server is already started.
     * @param galaxy_id string The galaxy ID
     * @param system_id string The system ID
     * @param sector_id string The sector ID
     */
    cls["start_sector"] = &Server::startSector;
    /**
     * @function Server:move_player_to_sector
     * Starts a sector. The function won't do anything if the server is already started.
     * @param server engine.Server The other server
     */
    cls["move_player_to_sector"] = &Server::movePlayerToSector;
}
