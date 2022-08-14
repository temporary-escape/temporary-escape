#include "Server.hpp"
#include "../Utils/Random.hpp"

#define CMP "Server"

using namespace Engine;

Server::Server(const Config& config, Registry& registry, TransactionalDatabase& db) :
    NetworkTcpServer(*this),
    config(config),
    registry(registry),
    db(db),
    world(config, registry, db),
    generator(config, db, world),
    tickFlag(true),
    worker(getWorker()),
    loader(1) {

    NetworkTcpServer<Server, ServerSink>::bind(config.serverPort);
    tickThread = std::thread(&Server::tick, this);
}

void Server::load() {
    loader.post([this]() {
        try {
            generator.generate(123456789ULL);
            Log::i(CMP, "Universe has been generated and is ready");

            loaded.resolve();
        } catch (std::exception& e) {
            BACKTRACE(CMP, e, "Failed to generate the universe");
            loaded.reject<std::runtime_error>(e.what());
        }
    });

    auto future = loaded.future();
    future.get();
}

Server::~Server() {
    Log::i(CMP, "Stopping");
    NetworkTcpServer<Server, ServerSink>::stop();
    tickFlag.store(false);
    tickThread.join();
    loader.stop();
    worker.stop();
}

void Server::tick() {
    Log::i(CMP, "Starting tick");

    Worker sectorWorker(4);

    while (tickFlag.load()) {
        const auto start = std::chrono::steady_clock::now();

        // TICK
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
        }

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
    std::unique_lock<std::shared_mutex> lock{players.mutex};
    players.lobby.insert(peer.get());
}

SessionPtr Server::peerToSession(const PeerPtr& peer) {
    std::unique_lock<std::shared_mutex> lock{players.mutex};
    const auto it = players.map.find(peer.get());
    if (it == players.map.end()) {
        EXCEPTION("Player session not found for peer: {}", reinterpret_cast<uint64_t>(peer.get()));
    }
    return it->second;
}

std::tuple<SessionPtr, SectorPtr> Server::peerToSessionSector(const PeerPtr& peer) {
    SessionPtr session;
    std::string sectorId;
    SectorPtr sector;

    {
        std::unique_lock<std::shared_mutex> lock{players.mutex};
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
        std::unique_lock<std::shared_mutex> lock{sectors.mutex};
        const auto it = sectors.map.find(sectorId);
        if (it == sectors.map.end()) {
            EXCEPTION("Player session present in invalid sector id: {} player id: {}", sectorId,
                      session->getPlayerId());
        }

        sector = it->second;
    }

    return {session, sector};
}

void Server::handle(const PeerPtr& peer, MessageLogin::Request req, MessageLogin::Response& res) {
    Log::i(CMP, "New login peer id: {}", reinterpret_cast<uint64_t>(peer.get()));

    // Check for server password
    if (!config.serverPassword.empty() && config.serverPassword != req.password) {
        res.error = "Bad server password";
        return;
    }

    // Handle lobby
    {
        std::unique_lock<std::shared_mutex> lock{players.mutex};
        const auto it = players.lobby.find(peer.get());
        if (it == players.lobby.end()) {
            res.error = "Peer connection failure";
            return;
        }
        players.lobby.erase(peer.get());
    }

    // Check if the player is already logged in
    const auto playerIdFound = world.playerSecretToId(req.secret);
    if (playerIdFound) {
        auto alreadyLoggedIn = false;
        {
            std::shared_lock<std::shared_mutex> lock{players.mutex};
            for (const auto& [_, other] : players.map) {
                if (other->getPlayerId() == playerIdFound.value()) {
                    alreadyLoggedIn = true;
                    break;
                }
            }
        }

        if (alreadyLoggedIn) {
            res.error = "Already logged in";
            return;
        }
    }

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
    });
}

void Server::handle(const PeerPtr& peer, MessageModsInfo::Request req, MessageModsInfo::Response& res) {
    /*const auto& manifests = modManager.getManifests();
    res.manifests.reserve(manifests.size());
    for (const auto& manifest : manifests) {
        res.manifests.push_back(*manifest);
    }*/
}

void Server::handle(const PeerPtr& peer, MessagePlayerLocation::Request req, MessagePlayerLocation::Response& res) {
    auto session = peerToSession(peer);
    res.location = world.getPlayerLocation(session->getPlayerId());
}

void Server::handle(const PeerPtr& peer, MessageFetchGalaxy::Request req, MessageFetchGalaxy::Response& res) {
    auto session = peerToSession(peer);
    res.galaxyId = req.galaxyId;
    res.galaxy = world.getGalaxyForPlayer(session->getPlayerId(), req.galaxyId);
}

void Server::handle(const PeerPtr& peer, MessageFetchSystems::Request req, MessageFetchSystems::Response& res) {
    auto session = peerToSession(peer);
    res.galaxyId = req.galaxyId;
    res.systems = world.getSystemsForPlayer(session->getPlayerId(), req.galaxyId, req.token, res.token);
}

void Server::handle(const PeerPtr& peer, MessageFetchRegions::Request req, MessageFetchRegions::Response& res) {
    auto session = peerToSession(peer);
    res.galaxyId = req.galaxyId;
    res.regions = world.getRegionsForPlayer(session->getPlayerId(), req.galaxyId, req.token, res.token);
}

void Server::handle(const PeerPtr& peer, MessageFetchFactions::Request req, MessageFetchFactions::Response& res) {
    auto session = peerToSession(peer);
    res.galaxyId = req.galaxyId;
    res.factions = world.getFactionsForPlayer(session->getPlayerId(), req.galaxyId, req.token, res.token);
}

void Server::handle(const PeerPtr& peer, MessageShipMovement::Request req, MessageShipMovement::Response& res) {
    auto [session, sector] = peerToSessionSector(peer);
    sector->handle(session, std::move(req), res);
}

void Server::handle(const PeerPtr& peer, MessageUnlockedBlocks::Request req, MessageUnlockedBlocks::Response& res) {
    auto session = peerToSession(peer);
    res.blocks = world.getPlayerUnlockedBlocks(session->getPlayerId());
}
