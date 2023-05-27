#include "server.hpp"
#include "../utils/random.hpp"
#include "lua.hpp"
#include <memory>
#include <sol/sol.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

#undef HANDLE_REQUEST
#define HANDLE_REQUEST(Req, Res)                                                                                       \
    Network::Server::addHandler([this](const PeerPtr& peer, Req req) -> Res {                                          \
        Res res{};                                                                                                     \
        this->handle(peer, std::move(req), res);                                                                       \
        return res;                                                                                                    \
    })
#undef HANDLE_REQUEST_VOID
#define HANDLE_REQUEST_VOID(Req)                                                                                       \
    Network::Server::addHandler([this](const PeerPtr& peer, Req req) -> void { this->handle(peer, std::move(req)); })

Server* Server::instance;

Server::Server(const Config& config, const Certs& certs, AssetsManager& assetsManager, Database& db) :
    Network::Server{static_cast<unsigned int>(config.serverPort), certs.key, certs.dh, certs.cert},
    config{config},
    assetsManager{assetsManager},
    db{db},
    playerSessions{config, assetsManager, db},
    world{config, assetsManager, db, playerSessions},
    lobby{config},
    tickFlag{true},
    worker{4},
    loadQueue{1} {

    instance = this;

    HANDLE_REQUEST(MessageLoginRequest, MessageLoginResponse);
    HANDLE_REQUEST(MessageModManifestsRequest, MessageModManifestsResponse);
    HANDLE_REQUEST(MessagePlayerLocationRequest, MessagePlayerLocationResponse);
    HANDLE_REQUEST_VOID(MessagePingResponse);
    world.registerHandlers(*this);

    auto promise = std::make_shared<Promise<void>>();
    tickThread = std::thread([this, promise]() {
        try {
            load();
        } catch (std::exception& e) {
            BACKTRACE(e, "Server load thread error");
            cleanup();
            promise->reject<std::runtime_error>("Server failed to start");
            return;
        }

        promise->resolve();

        try {
            tick();
        } catch (std::exception& e) {
            BACKTRACE(e, "Server tick thread error");
        }
        cleanup();
    });

    try {
        auto future = promise->future();
        future.get();
    } catch (...) {
        tickThread.join();
        throw;
    }
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
        lua->importModule("base");
    } catch (...) {
        EXCEPTION_NESTED("Failed to import base assets module");
    }

    try {
        generator(123456789LL);
        logger.info("Universe has been generated and is ready");
    } catch (...) {
        EXCEPTION_NESTED("Failed to generate the universe");
    }

    try {
        Network::Server::start();
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
    Network::Server::stop();

    logger.info("Waiting for load queue to stop");
    loadQueue.stop();

    logger.info("Waiting for workers to stop");
    worker.stop();

    logger.info("Clearing sectors");
    {
        std::unique_lock<std::shared_mutex> lock{sectors.mutex};
        sectors.map.clear();
    }

    logger.info("Stopping event bus");
    eventBus.reset();

    logger.info("Stopping lua");
    generator = nullptr;
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

        playerSessions.updateSessionsPing();
        pollEvents();
        updateSectors();

        const auto now = std::chrono::steady_clock::now();
        const auto test = std::chrono::duration_cast<std::chrono::microseconds>(now - start);
        if (test < config.tickLengthUs) {
            std::this_thread::sleep_for(config.tickLengthUs - test);
        }
    }

    logger.info("Stopped tick");
}

void Server::onAcceptSuccess(PeerPtr peer) {
    logger.info("New connection peer id: {}", reinterpret_cast<uint64_t>(peer.get()));
    lobby.addPeerToLobby(peer);
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
        loadQueue.postSafe([this, sectorPtr]() { sectorPtr->load(); });

        return sectorPtr;
    } catch (...) {
        EXCEPTION_NESTED("Failed to start sector '{}'", sector.id);
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

void Server::handle(const PeerPtr& peer, MessageLoginRequest req, MessageLoginResponse& res) {
    logger.info("New login peer id: {}", reinterpret_cast<uint64_t>(peer.get()));

    // Check for server password
    if (!config.serverPassword.empty() && config.serverPassword != req.password) {
        res.error = "Bad server password";
        lobby.disconnectPeer(peer);
        return;
    }

    // Check if the player is already logged in
    const auto playerIdFound = playerSessions.secretToId(req.secret);
    if (playerIdFound) {
        if (playerSessions.isLoggedIn(*playerIdFound)) {
            res.error = "Already logged in";
            lobby.disconnectPeer(peer);
            return;
        }
    }

    // Remove peer from lobby
    lobby.removePeerFromLobby(peer);

    // Login
    logger.info("Logging in player name: '{}'", req.name);

    PlayerData player;
    try {
        player = playerSessions.login(req.secret, req.name);
    } catch (...) {
        res.error = "Failed logging in";
        lobby.disconnectPeer(peer);
        EXCEPTION_NESTED("Failed to log in player: '{}'", req.name);
    }

    logger.info("Logged in player: {} name: '{}'", player.id, req.name);

    playerSessions.createSession(peer, player.id);

    // Publish an event
    EventData event{};
    event["player_id"] = player.id;
    event["player_name"] = player.name;
    eventBus->enqueue("player_logged_in", event);
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

void Server::handle(const PeerPtr& peer, MessageModManifestsRequest req, MessageModManifestsResponse& res) {
    (void)peer;
    (void)req;

    const auto& manifests = assetsManager.getManifests();
    res.items.reserve(manifests.size());
    for (const auto& manifest : manifests) {
        res.items.push_back(manifest);
    }
    res.page.hasNext = false;
}

void Server::handle(const PeerPtr& peer, MessagePlayerLocationRequest req, MessagePlayerLocationResponse& res) {
    (void)req;

    const auto session = playerSessions.getSession(peer);
    const auto location = db.find<PlayerLocationData>(session->getPlayerId());
    if (location) {
        res.location = *location;
    } else {
        logger.warn("Player location requested for: '{}' but player has no location", session->getPlayerId());
    }
}

void Server::handle(const PeerPtr& peer, MessagePingResponse res) {
    (void)res;

    auto session = playerSessions.getSession(peer);
    session->setLastPingTime(std::chrono::steady_clock::now());
    session->clearFlag(Session::Flags::PingSent);
}

void Server::onError(std::error_code ec) {
    logger.error("Server network error: {} ({})", ec.message(), ec.category().name());
}

void Server::onError(const PeerPtr& peer, std::error_code ec) {
    logger.error("Server network error: {} ({})", ec.message(), ec.category().name());
    peer->close();
}

void Server::onUnhandledException(const PeerPtr& peer, std::exception_ptr& eptr) {
    try {
        std::rethrow_exception(eptr);
    } catch (std::exception& e) {
        BACKTRACE(e, "Server network error");
    }
    peer->close();
}

void Server::postDispatch(std::function<void()> fn) {
    worker.postSafe(std::forward<decltype(fn)>(fn));
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
    cls["set_generator"] = [](Server& self, sol::function fn) {
        self.setGenerator([fn](uint64_t seed) {
            sol::protected_function_result result = fn(seed);
            if (!result.valid()) {
                sol::error err = result;
                EXCEPTION("{}", err.what());
            }
        });
    };
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
