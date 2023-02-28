#include "server.hpp"
#include "../utils/random.hpp"
#include "generator_default.hpp"
#include "python.hpp"
#include <memory>

using namespace Engine;

static auto logger = createLogger(__FILENAME__);

#undef HANDLE_REQUEST
#define HANDLE_REQUEST(Req, Res)                                                                                       \
    Network::Server::addHandler([this](const PeerPtr& peer, Req req) -> Res {                                          \
        Res res{};                                                                                                     \
        this->handle(peer, std::move(req), res);                                                                       \
        return res;                                                                                                    \
    });
#define HANDLE_REQUEST_VOID(Req)                                                                                       \
    Network::Server::addHandler([this](const PeerPtr& peer, Req req) -> void { this->handle(peer, std::move(req)); });

Server::Server(const Config& config, const Certs& certs, Registry& registry, TransactionalDatabase& db) :
    Network::Server{static_cast<unsigned int>(config.serverPort), certs.key, certs.dh, certs.cert},
    config{config},
    registry{registry},
    world{config, registry, db, *this, *this},
    generator{std::make_unique<GeneratorDefault>(config, world)},
    tickFlag{true},
    python{std::make_unique<Python>(config.pythonHome, std::vector<Path>{config.assetsPath})},
    worker{4},
    commands{worker.strand()} {

    HANDLE_REQUEST(MessageLoginRequest, MessageLoginResponse);
    HANDLE_REQUEST(MessageModsInfoRequest, MessageModsInfoResponse);
    HANDLE_REQUEST(MessageSpawnRequest, MessageSpawnResponse);
    HANDLE_REQUEST_VOID(MessagePingResponse);
}

void Server::load() {
    try {
        generator->generate(123456789ULL);
        tickThread = std::thread(&Server::tick, this);
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
}

void Server::stop() {
    logger.info("Stopping");

    {
        std::unique_lock<std::shared_mutex> lock{players.mutex};
        players.lobby.clear();
        players.sessions.clear();
    }

    tickFlag.store(false);
    if (tickThread.joinable()) {
        tickThread.join();
    }
    worker.stop();
    Network::Server::stop();
}

Server::~Server() {
    stop();
}

void Server::tick() {
    logger.info("Starting tick");

    while (tickFlag.load()) {
        const auto start = std::chrono::steady_clock::now();

        const auto sessions = getAllSessions();
        updateSessionsPing(sessions);

        /*// TICK
        std::optional<std::string> failedId;
        {
            std::shared_lock<std::shared_mutex> lock{sectors.mutex};
            for (auto& pair : sectors.map) {
                const auto& compoundId = pair.first;
                auto& sector = pair.second;
                sectorWorker.post([&failedId, sector, compoundId, this]() {
                    try {
                        sector->update(config.tickLengthUs.count() / 1000000.0f);
                    } catch (std::exception& e) {
                        BACKTRACE(CMP, e, "Failed to update sector");
                        failedId = compoundId;
                    }
                });
            }
        }

        sectorWorker.run();

        if (failedId.has_value()) {
            logger.error("Sector: '{}' failed to update, stopping tick", failedId.value());
            break;
        }*/

        const auto now = std::chrono::steady_clock::now();
        const auto test = std::chrono::duration_cast<std::chrono::microseconds>(now - start);
        if (test < config.tickLengthUs) {
            std::this_thread::sleep_for(config.tickLengthUs - test);
        }
    }

    logger.info("Stopped tick");
}

void Server::updateSessionsPing(const std::vector<SessionPtr>& sessions) {
    const auto now = std::chrono::steady_clock::now();
    for (const auto& session : sessions) {
        if (!session->hasFlag(Session::Flags::PingSent)) {
            if (now - session->getLastPingTime() > std::chrono::milliseconds{250}) {
                session->setFlag(Session::Flags::PingSent);

                MessagePingRequest req{};
                req.time = now;
                session->send(req);
            }
        }
    }
}

void Server::onAcceptSuccess(PeerPtr peer) {
    logger.info("New connection peer id: {}", reinterpret_cast<uint64_t>(peer.get()));
    addPeerToLobby(peer);
}

SessionPtr Server::peerToSession(const PeerPtr& peer) {
    std::shared_lock<std::shared_mutex> lock{players.mutex};
    const auto it = players.sessions.find(peer.get());
    if (it == players.sessions.end()) {
        EXCEPTION("Player session not found for peer: {}", reinterpret_cast<uint64_t>(peer.get()));
    }
    return it->second;
}

bool Server::isPeerLoggedIn(const std::string& playerId) {
    std::shared_lock<std::shared_mutex> lock{players.mutex};
    for (const auto& [_, session] : players.sessions) {
        if (session->getPlayerId() == playerId) {
            return true;
        }
    }

    return false;
}

void Server::addPeerToLobby(const PeerPtr& peer) {
    logger.info("Adding to lobby peer id: {}", reinterpret_cast<uint64_t>(peer.get()));
    std::unique_lock<std::shared_mutex> lock{players.mutex};
    players.lobby.insert(peer.get());
}

void Server::removePeerFromLobby(const PeerPtr& peer) {
    std::unique_lock<std::shared_mutex> lock{players.mutex};
    players.lobby.erase(peer.get());

    const auto it = players.lobby.find(peer.get());
    if (it != players.lobby.end()) {
        logger.info("Removing from lobby peer id: {}", reinterpret_cast<uint64_t>(peer.get()));
        players.lobby.erase(it);
    }
}

void Server::disconnectPeer(const PeerPtr& peer) {
    logger.info("Disconnecting peer id: {}", reinterpret_cast<uint64_t>(peer.get()));
    removePeerFromLobby(peer);
    peer->close();
}

SessionPtr Server::createSession(const PeerPtr& peer, const PlayerData& player) {
    logger.info("Creating session for peer id: {} player: {}", reinterpret_cast<uint64_t>(peer.get()), player.id);
    std::unique_lock<std::shared_mutex> lock{players.mutex};
    auto session = std::make_shared<Session>(player.id, peer);
    players.sessions.insert(std::make_pair(peer.get(), session));
    return session;
}

SectorPtr Server::startSector(const std::string& galaxyId, const std::string& systemId, const std::string& sectorId) {
    logger.info("Starting sector: '{}/{}/{}'", galaxyId, systemId, sectorId);

    auto sectorOpt = world.sectors.find(galaxyId, systemId, sectorId);
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
        auto instance = std::make_shared<Sector>(config, world, registry, sector.galaxyId, sector.systemId, sector.id);
        sectors.map.insert(std::make_pair(sector.id, instance));

        return instance;
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

    auto& sector = *it->second;

    MessagePlayerLocationChanged msg{};
    msg.location.galaxyId = sector.getGalaxyId();
    msg.location.systemId = sector.getSystemId();
    msg.location.sectorId = sector.getSectorId();

    session->send(msg);
}

std::tuple<SessionPtr, SectorPtr> Server::peerToSessionSector(const PeerPtr& peer) {
    /*SessionPtr session;
    std::string sectorId;
    SectorPtr sector;

    {
        std::shared_lock<std::shared_mutex> lock{players.mutex};
        const auto it = players.map.find(peer.get());
        if (it == players.map.end()) {
            EXCEPTION("Player session not found for peer: {}", reinterpret_cast<uint64_t>(peer.get()));
        }
        session = it->second;

        const auto it2 = players.sectors.find(session.get());
        if (it2 == players.sectors.end()) {
            EXCEPTION("Player session not present in any sector, player id: {}", session->getPlayerId());
        }

        sectorId = it2->second;
    }

    {
        std::shared_lock<std::shared_mutex> lock{sectors.mutex};
        const auto it = sectors.map.find(sectorId);
        if (it == sectors.map.end()) {
            EXCEPTION("Player session present in invalid sector id: {} player id: {}", sectorId,
                      session->getPlayerId());
        }

        sector = it->second;
    }

    return {session, sector};*/

    return {nullptr, nullptr};
}

std::vector<SessionPtr> Server::getAllSessions() {
    std::shared_lock<std::shared_mutex> lock{players.mutex};
    std::vector<SessionPtr> sessions;
    sessions.reserve(players.sessions.size());
    for (const auto& [_, peer] : players.sessions) {
        sessions.push_back(peer);
    }
    return sessions;
}

void Server::handle(const PeerPtr& peer, MessageLoginRequest req, MessageLoginResponse& res) {
    logger.info("New login peer id: {}", reinterpret_cast<uint64_t>(peer.get()));

    // Check for server password
    if (!config.serverPassword.empty() && config.serverPassword != req.password) {
        res.error = "Bad server password";
        disconnectPeer(peer);
        return;
    }

    // Remove peer from lobby
    removePeerFromLobby(peer);

    // Check if the player is already logged in
    const auto playerIdFound = world.players.secretToId(req.secret);
    if (playerIdFound) {
        if (isPeerLoggedIn(*playerIdFound)) {
            res.error = "Already logged in";
            disconnectPeer(peer);
            return;
        }
    }

    // Login
    logger.info("Logging in player name: '{}'", req.name);

    PlayerData player;
    try {
        player = world.players.login(req.secret, req.name);
    } catch (...) {
        res.error = "Failed logging in";
        disconnectPeer(peer);
        EXCEPTION_NESTED("Failed to log in player: '{}'", req.name);
    }

    logger.info("Logged in player: {} name: '{}'", player.id, req.name);

    createSession(peer, player);

    /*

    // Login
    logger.info("Logging in player: '{}'", req.name);

    PlayerData player;
    SessionPtr session;
    try {
        player = world.loginPlayer(req.secret, req.name);
        session = std::make_shared<Session>(player.id, peer);

        std::unique_lock<std::shared_mutex> lock{players.mutex};
        players.map.insert(std::make_pair(peer.get(), session));
    } catch (...) {
        EXCEPTION_NESTED("Failed to log in player: '{}'", req.name);
    }

    res.playerId = player.id;

    // Find starting location, so we can move the player to the correct sector
    PlayerLocationData location;
    try {
        location = world.findPlayerStartingLocation(player.id);
    } catch (...) {
        EXCEPTION_NESTED("Failed to find starting location for player: '{}'", req.name);
    }

    // Find the sector
    auto sectorOpt = world.findSector(location.galaxyId, location.systemId, location.sectorId);
    if (!sectorOpt) {
        EXCEPTION_NESTED("Invalid starting location for player: '{}'", req.name);
    }
    auto& sector = sectorOpt.value();

    // Start the sector
    std::shared_ptr<Sector> instance;
    {
        std::shared_lock<std::shared_mutex> lock{sectors.mutex};
        auto it = sectors.map.find(sector.id);
        if (it == sectors.map.end()) {
            try {
                instance =
                    std::make_shared<Sector>(config, world, registry, db, sector.galaxyId, sector.systemId, sector.id);
                it = sectors.map.insert(std::make_pair(sector.id, instance)).first;
            } catch (...) {
                EXCEPTION_NESTED("Failed to start sector '{}'", sector.id);
            }
        }
    }

    {
        std::unique_lock<std::shared_mutex> lock{players.mutex};
        players.sectors.insert(std::make_pair(session.get(), sector.id));
    }

    loader.post([this, instance, session, sector]() {
        try {
            instance->load();
            instance->addPlayer(session);
        } catch (std::exception& e) {
            BACKTRACE(CMP, e, "Failed to load sector id: '{}'", sector.id);
        }
    });*/
}

void Server::handle(const PeerPtr& peer, MessageSpawnRequest req, MessageSpawnResponse& res) {
    auto session = peerToSession(peer);
    try {
        const auto location = world.players.findStartingLocation(session->getPlayerId());
        res.location = location;

        startSector(location.galaxyId, location.systemId, location.sectorId);
        addPlayerToSector(session, location.sectorId);
    } catch (...) {
        EXCEPTION_NESTED("Failed to find starting location for player: '{}'", session->getPlayerId());
    }
}

void Server::handle(const PeerPtr& peer, MessageModsInfoRequest req, MessageModsInfoResponse& res) {
    const auto& manifests = registry.getManifests();
    res.manifests.reserve(manifests.size());
    for (const auto& manifest : manifests) {
        res.manifests.push_back(manifest);
    }
}

void Server::handle(const PeerPtr& peer, MessageShipMovementRequest req, MessageShipMovementResponse& res) {
    auto [session, sector] = peerToSessionSector(peer);
    // sector->handle(session, std::move(req), res);
}

void Server::handle(const PeerPtr& peer, MessagePingResponse res) {
    auto session = peerToSession(peer);
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

std::shared_ptr<Service::Session> Server::find(const std::shared_ptr<Network::Peer>& peer) {
    return peerToSession(peer);
}
