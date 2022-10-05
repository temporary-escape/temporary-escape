#include "Server.hpp"
#include "../Utils/Random.hpp"

#define CMP "Server"

using namespace Engine;

Server::Server(const Config& config, Registry& registry, TransactionalDatabase& db) :
    NetworkTcpServer(*this, 1),
    config(config),
    registry(registry),
    db(db),
    world(config, registry, db),
    generator(config, db, world),
    tickFlag(true),
    commands{getWorker().strand()} {

    NetworkTcpServer<Server, ServerSink>::bind(config.serverPort);
}

Future<void> Server::load() {
    return std::async([this]() {
        try {
            generator.generate(123456789ULL);
            tickThread = std::thread(&Server::tick, this);
            Log::i(CMP, "Universe has been generated and is ready");
        } catch (...) {
            EXCEPTION_NESTED("Failed to generate the universe");
        }
    });
}

Server::~Server() {
    Log::i(CMP, "Stopping");
    NetworkTcpServer<Server, ServerSink>::stop();
    tickFlag.store(false);
    if (tickThread.joinable()) {
        tickThread.join();
    }
}

void Server::tick() {
    Log::i(CMP, "Starting tick");

    Worker workers(4);

    while (tickFlag.load()) {
        const auto start = std::chrono::steady_clock::now();

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
            Log::e(CMP, "Sector: '{}' failed to update, stopping tick", failedId.value());
            break;
        }*/

        const auto now = std::chrono::steady_clock::now();
        const auto test = std::chrono::duration_cast<std::chrono::microseconds>(now - start);
        if (test < config.tickLengthUs) {
            std::this_thread::sleep_for(config.tickLengthUs - test);
        }
    }

    Log::i(CMP, "Stopped tick");
}

void Server::onPeerConnected(NetworkTcpServer::PeerPtr peer) {
    Log::i(CMP, "New connection peer id: {}", reinterpret_cast<uint64_t>(peer.get()));
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

void Server::addPeerToLobby(const NetworkTcpServer::PeerPtr& peer) {
    Log::i(CMP, "Adding to lobby peer id: {}", reinterpret_cast<uint64_t>(peer.get()));
    std::unique_lock<std::shared_mutex> lock{players.mutex};
    players.lobby.insert(peer.get());
}

void Server::removePeerFromLobby(const NetworkTcpServer::PeerPtr& peer) {
    std::unique_lock<std::shared_mutex> lock{players.mutex};
    players.lobby.erase(peer.get());

    const auto it = players.lobby.find(peer.get());
    if (it != players.lobby.end()) {
        Log::i(CMP, "Removing from lobby peer id: {}", reinterpret_cast<uint64_t>(peer.get()));
        players.lobby.erase(it);
    }
}

void Server::disconnectPeer(const NetworkTcpServer::PeerPtr& peer) {
    Log::i(CMP, "Disconnecting peer id: {}", reinterpret_cast<uint64_t>(peer.get()));
    removePeerFromLobby(peer);
    peer->close();
}

SessionPtr Server::createSession(const PeerPtr& peer, const PlayerData& player) {
    Log::i(CMP, "Creating session for peer id: {} player: {}", reinterpret_cast<uint64_t>(peer.get()), player.id);
    std::unique_lock<std::shared_mutex> lock{players.mutex};
    auto session = std::make_shared<Session>(player.id, peer);
    players.sessions.insert(std::make_pair(peer.get(), session));
    return session;
}

SectorPtr Server::startSector(const std::string& galaxyId, const std::string& systemId, const std::string& sectorId) {
    Log::i(CMP, "Starting sector: '{}/{}/{}'", galaxyId, systemId, sectorId);

    auto sectorOpt = world.sectors.find(galaxyId, systemId, sectorId);
    if (!sectorOpt) {
        EXCEPTION_NESTED("Can not start sector: '{}/{}/{}' not found", galaxyId, systemId, sectorId);
    }
    auto& sector = sectorOpt.value();

    std::unique_lock<std::shared_mutex> lock{sectors.mutex};

    auto it = sectors.map.find(sector.id);
    if (it != sectors.map.end()) {
        Log::i(CMP, "Starting sector: '{}/{}/{}' skipped, already started", galaxyId, systemId, sectorId);
        return it->second;
    }

    try {
        Log::i(CMP, "Creating sector: '{}/{}/{}'", galaxyId, systemId, sectorId);
        auto instance =
            std::make_shared<Sector>(config, world, registry, db, sector.galaxyId, sector.systemId, sector.id);
        sectors.map.insert(std::make_pair(sector.id, instance));

        return instance;
    } catch (...) {
        EXCEPTION_NESTED("Failed to start sector '{}'", sector.id);
    }
}

void Server::addPlayerToSector(const SessionPtr& session, const std::string& sectorId) {
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

void Server::handle(const PeerPtr& peer, MessageLogin::Request req, MessageLogin::Response& res) {
    Log::i(CMP, "New login peer id: {}", reinterpret_cast<uint64_t>(peer.get()));

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
    Log::i(CMP, "Logging in player name: '{}'", req.name);

    PlayerData player;
    try {
        player = world.players.login(req.secret, req.name);
    } catch (...) {
        res.error = "Failed logging in";
        disconnectPeer(peer);
        EXCEPTION_NESTED("Failed to log in player: '{}'", req.name);
    }

    Log::i(CMP, "Logged in player: {} name: '{}'", player.id, req.name);

    createSession(peer, player);

    /*

    // Login
    Log::i(CMP, "Logging in player: '{}'", req.name);

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

void Server::handle(const PeerPtr& peer, MessageRequestSpawn::Request req, MessageRequestSpawn::Response& res) {
    auto session = peerToSession(peer);
    try {
        const auto location = world.players.findStartingLocation(session->getPlayerId());
        res.location = location;

        commands.postSafe([=]() {
            startSector(location.galaxyId, location.systemId, location.sectorId);
            addPlayerToSector(session, location.sectorId);
        });
    } catch (...) {
        EXCEPTION_NESTED("Failed to find starting location for player: '{}'", session->getPlayerId());
    }
}

void Server::handle(const PeerPtr& peer, MessageModsInfo::Request req, MessageModsInfo::Response& res) {
    const auto& manifests = registry.getManifests();
    res.manifests.reserve(manifests.size());
    for (const auto& manifest : manifests) {
        res.manifests.push_back(manifest);
    }
}

void Server::handle(const PeerPtr& peer, MessagePlayerLocation::Request req, MessagePlayerLocation::Response& res) {
    auto session = peerToSession(peer);
    res.location = world.players.getLocation(session->getPlayerId());
}

void Server::handle(const PeerPtr& peer, MessageFetchGalaxy::Request req, MessageFetchGalaxy::Response& res) {
    auto session = peerToSession(peer);
    res.galaxyId = req.galaxyId;
    res.galaxy = world.galaxies.getForPlayer(session->getPlayerId(), req.galaxyId);
}

void Server::handle(const PeerPtr& peer, MessageFetchSystems::Request req, MessageFetchSystems::Response& res) {
    auto session = peerToSession(peer);
    res.galaxyId = req.galaxyId;
    res.systems = world.systems.getForPlayer(session->getPlayerId(), req.galaxyId, req.token, res.token);
}

void Server::handle(const PeerPtr& peer, MessageFetchRegions::Request req, MessageFetchRegions::Response& res) {
    auto session = peerToSession(peer);
    res.galaxyId = req.galaxyId;
    res.regions = world.regions.getForPlayer(session->getPlayerId(), req.galaxyId, req.token, res.token);
}

void Server::handle(const PeerPtr& peer, MessageFetchFactions::Request req, MessageFetchFactions::Response& res) {
    auto session = peerToSession(peer);
    res.galaxyId = req.galaxyId;
    res.factions = world.factions.getForPlayer(session->getPlayerId(), req.galaxyId, req.token, res.token);
}

void Server::handle(const PeerPtr& peer, MessageShipMovement::Request req, MessageShipMovement::Response& res) {
    auto [session, sector] = peerToSessionSector(peer);
    sector->handle(session, std::move(req), res);
}

void Server::handle(const PeerPtr& peer, MessageUnlockedBlocks::Request req, MessageUnlockedBlocks::Response& res) {
    auto session = peerToSession(peer);
    res.blocks = world.players.getUnlockedBlocks(session->getPlayerId());
}
