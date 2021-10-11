#include "Server.hpp"

#include "../Network/NetworkTcpAcceptor.hpp"
#include "../Utils/Random.hpp"
#include "Generator.hpp"

#include <random>

using namespace Scissio;

static const double TickLength = 1.0f / 20.0f;
static const auto TickLengthUs = std::chrono::microseconds(static_cast<long long>(TickLength * 1000.0 * 1000.0));

#define DISPATCH_FUNC(M, T, F)                                                                                         \
    std::bind(static_cast<void (T::*)(PlayerSessionPtr, M)>(&T::F), this, std::placeholders::_1, std::placeholders::_2)
#define MESSAGE_DISPATCH(M) dispatcher.add<M>(DISPATCH_FUNC(M, Server, handle));

#define STRAND(...)                                                                                                    \
    try {                                                                                                              \
        strand.post(__VA_ARGS__);                                                                                      \
    } catch (std::exception & e) {                                                                                     \
        BACKTRACE(e, "async work error");                                                                              \
    }

#define WORK(...)                                                                                                      \
    try {                                                                                                              \
        strand.post(__VA_ARGS__);                                                                                      \
    } catch (std::exception & e) {                                                                                     \
        BACKTRACE(e, "async work error");                                                                              \
    }

#define LOADER(...)                                                                                                    \
    try {                                                                                                              \
        loader.post(__VA_ARGS__);                                                                                      \
    } catch (std::exception & e) {                                                                                     \
        BACKTRACE(e, "async work error");                                                                              \
    }

Server::Server(const Config& config, AssetManager& assetManager, Database& db)
    : config(config), assetManager(assetManager), db(db), world(config, db), worker(4), strand(worker.strand()),
      tickFlag(true) {

    // MESSAGE_DISPATCH(MessageHelloRequest);
    // MESSAGE_DISPATCH(MessageLoginRequest);
    MESSAGE_DISPATCH(MessageSystemsRequest);
    MESSAGE_DISPATCH(MessageRegionsRequest);
    MESSAGE_DISPATCH(MessageBlocksRequest);
    MESSAGE_DISPATCH(MessageSectorStatusRequest);

    bind<Network::TcpAcceptor>(config.serverPort);

    tickThread = std::thread(&Server::tick, this);
}

Server::~Server() {
    tickFlag.store(false);
    tickThread.join();
}

void Server::load() {
    const auto seed = 123456789ULL;

    if (!world.galaxies.get(1).has_value()) {
        Log::i("Generating universe with seed {}", seed);

        const auto galaxy = Generator::generateGalaxy(config, world, seed);
        (void)galaxy;
    }
}

void Server::tick() {
    while (tickFlag.load()) {
        const auto start = std::chrono::steady_clock::now();

        {
            // std::unique_lock<std::shared_mutex> lock{glock};

            auto failed = false;

            // By doing it as strand, this ensures that
            // any events that occur in this server class must not be run parallel with
            // zone's tick!
            // For example, when we want to move the player from one zone to another,
            // we would like to avoid locks. When the move happens, the cache changes and
            // zone's list of players changes. To avoid lock, these events run before
            // the tick. However, the tick will run in parallel. One zone per thread, until the
            // number of available threads is exhausted.
            strand.post([this, &failed]() {
                for (const auto& [sectorId, zone] : zones) {
                    if (!zone->isReady()) {
                        continue;
                    }

                    // Uses its own strand, this call will not block
                    try {
                        zone->tick();
                    } catch (std::exception& e) {
                        failed = true;
                        BACKTRACE(e, "async tick failed");
                    }
                }
            });

            // This is where the real tick for each zone is performed
            worker.run();

            if (failed) {
                Log::e("Tick failed, stopping server");
                tickFlag.store(false);
            }
        }

        const auto now = std::chrono::steady_clock::now();
        const auto test = std::chrono::duration_cast<std::chrono::microseconds>(now - start);
        if (test < TickLengthUs) {
            std::this_thread::sleep_for(TickLengthUs - test);
        }
    }
}

void Server::handle(PlayerSessionPtr session, MessageHelloRequest req) {
    (void)req;

    WORK([=]() {
        MessageHelloResponse res;
        res.name = "Server Name";
        res.version = {1, 0, 0};
        session->send(res);
    });
}

void Server::login(const Network::StreamPtr& stream, MessageLoginRequest req) {
    (void)req;

    STRAND([=]() {
        // Remove from the lobby
        {
            std::unique_lock<std::shared_mutex> lock{playersMutex};
            lobby.erase(std::find(lobby.begin(), lobby.end(), stream));
        }

        // Login or create player
        auto player = world.playerLogin(req.uid, req.name);
        if (!player.has_value()) {
            if (world.playerNameTaken(req.name)) {
                MessageLoginResponse res{};
                res.error = "Name already taken";
                stream->send(res);
                return;
            }

            player = world.playerRegister(req.uid, req.name);
        }

        const auto session = std::make_shared<PlayerSession>(stream, player->id);

        // Init player info
        world.playerInit(req.uid);

        // Load or create zone based on player location
        const auto location = world.players.getLocation(player->id);
        if (location) {
            auto it = zones.find(location->sectorId);
            if (it == zones.end()) {
                const auto sector = world.sectors.get(location->sectorId);

                const auto ptr = std::make_shared<Zone>(config, assetManager, world, worker.strand(), sector->id);
                it = zones.insert(std::make_pair(location->sectorId, ptr)).first;
                const auto zone = it->second;

                LOADER([zone]() { zone->load(); });
            }

            it->second->addPlayer(session);

        } else {
            Log::e("Player {} has no starting location, possibly a bug", player->id);
            // TODO: disconnect
        }

        // Player cache
        {
            std::unique_lock<std::shared_mutex> lock{playersMutex};
            streamToPlayer.insert(std::make_pair(session->getStream(), session));
            players.insert(std::make_pair(player->id, session));
        }
    });
}

void Server::handle(PlayerSessionPtr session, MessageSystemsRequest req) {
    WORK([=]() {
        MessageSystemsResponse res = world.systems.findForPlayer(session->getPlayerId(), req.cont);
        session->send(res);
    });
}

void Server::handle(PlayerSessionPtr session, MessageRegionsRequest req) {
    WORK([=]() {
        MessageRegionsResponse res = world.regions.findForPlayer(session->getPlayerId(), req.cont);
        session->send(res);
    });
}

void Server::handle(PlayerSessionPtr session, MessageBlocksRequest req) {
    WORK([=]() {
        MessageBlocksResponse res = world.blocks.findForPlayer(session->getPlayerId(), req.cont);
        session->send(res);
    });
}

void Server::handle(PlayerSessionPtr session, MessageSectorStatusRequest req) {
    (void)req;

    STRAND([=]() {
        MessageSectorStatusResponse res{false};

        const auto zone = findZoneByPlayer(session);

        if (zone.has_value() && zone.value()->isReady()) {
            res.loaded = true;

            STRAND([=]() { zone.value()->addPlayer(session); });
        }

        session->send(res);
    });
}

void Server::dispatch(PlayerSessionPtr session, const Network::Packet packet) {
    try {
        dispatcher.dispatch(std::move(session), packet);
    } catch (...) {
        EXCEPTION_NESTED("Failed to dispatch message");
    }
}

void Server::eventConnect(const Network::StreamPtr& stream) {
    STRAND([=]() {
        std::unique_lock<std::shared_mutex> lock{playersMutex};

        lobby.push_back(stream);
    });
}

void Server::eventDisconnect(const Network::StreamPtr& stream) {
    STRAND([=]() {
        std::unique_lock<std::shared_mutex> lock{playersMutex};

        lobby.erase(std::find(lobby.begin(), lobby.end(), stream));

        const auto player = streamToPlayer.find(stream);
        if (player != streamToPlayer.end()) {
            streamToPlayer.erase(stream);
            players.erase(player->second->getPlayerId());
        }

        const auto zone = playerToZone.find(player->second->getPlayerId());
        if (zone != playerToZone.end()) {
            zone->second->removePlayer(player->second);
            playerToZone.erase(zone);
        }
    });
}

void Server::eventPacket(const Network::StreamPtr& stream, Network::Packet packet) {
    try {
        if (packet.id == Network::getMessageId<MessageLoginRequest>()) {
            login(stream, Network::unpack<MessageLoginRequest>(packet));
        } else {
            auto session = findPlayerByStream(stream);
            if (!session) {
                MessageServerError err{};
                err.msg = "No session established";
                stream->send(err);
                stream->disconnect();
            } else {
                dispatch(session.value(), std::move(packet));
            }
        }
    } catch (std::exception& e) {
        BACKTRACE(e, "Client failed to process packet");
    }
}

std::optional<PlayerSessionPtr> Server::findPlayerByStream(const Network::StreamPtr& stream) {
    std::shared_lock<std::shared_mutex> lock{playersMutex};

    const auto it = streamToPlayer.find(stream);
    if (it == streamToPlayer.end()) {
        return std::nullopt;
    }

    return it->second;
}

/*// Requires lock by caller
uint64_t Server::createSessionId() const {
    return randomId([&](uint64_t test) -> bool {
        const auto pred = [&](const PlayerSessionPtr& p) { return p->getSessionId() == test; };
        return std::find_if(players.begin(), players.end(), pred) == players.end();
    });
}

void Server::preparePlayer(const PlayerSessionPtr& session, const Player& player) {
    Log::v("Preparing player: '{}' ({})", player.name, player.id);

    world.playerInit(player.id);

    const auto location = world.players.getLocation(player.id);
    if (location.has_value()) {
        const auto sector = world.sectors.get(location.value().sectorId);

        auto w = [=]() { prepareZone(sector.value()); };

        WORK_SAFE(strand, w);

        playerZoneCache.insert(std::make_pair(player.id, location.value().sectorId));

        const MessageSectorChanged msg{
            1,
            sector.value().systemId,
            SectorDto::from(sector.value()),
        };

        session->send(0, msg);
    }
}

void Server::prepareZone(const Sector& sector) {
    Log::v("Preparing sector: '{}' ({})", sector.name, sector.id);

    const auto found = findZoneById(sector.id);
    if (found.has_value()) {
        Log::v("Sector: {} is already alive", sector.id);
        return;
    }

    const auto zone = std::make_shared<Zone>(config, assetManager, world, worker.strand(), sector.id);
    zones.insert(std::make_pair(sector.id, zone));

    auto w = [=]() { zone->load(); };

    WORK_SAFE(loader, w);
}

std::optional<ZonePtr> Server::findZoneById(const uint64_t id) {
    const auto it = zones.find(id);
    if (it == zones.end()) {
        return std::nullopt;
    }

    return it->second;
}

std::optional<ZonePtr> Server::findZoneByPlayer(const uint64_t id) {
    const auto it = playerZoneCache.find(id);
    if (it == playerZoneCache.end()) {
        return std::nullopt;
    }

    return findZoneById(it->second);
}

std::optional<ZonePtr> Server::findZoneByPlayer(const PlayerSessionPtr& session) {
    return findZoneByPlayer(session->getPlayerId());
}

Network::SessionPtr Server::createSession(const uint64_t uid, const std::string& name, const std::string& password) {
    std::unique_lock<std::shared_mutex> lock{glock};

    // Login, throws on error
    const auto player = world.playerLogin(uid, name);

    const auto sessionId = createSessionId();
    const auto playerSession = std::make_shared<PlayerSession>(sessionId, uid);

    Log::i("Creating network session for player uid: {}", playerSession->getPlayerId());
    playersLobby.push_back(playerSession);

    auto w = [=]() { preparePlayer(playerSession, player); };

    WORK_SAFE(strand, w);

    return playersLobby.back();
}

void Server::acceptSession(const Network::SessionPtr& session) {
    std::unique_lock<std::shared_mutex> lock{glock};

    auto player = std::dynamic_pointer_cast<PlayerSession>(session);

    const auto it = std::find(playersLobby.begin(), playersLobby.end(), player);
    if (it != playersLobby.end()) {
        playersLobby.erase(it);

        Log::i("Accepting network session for player uid: {}", player->getPlayerId());
        players.push_back(std::move(player));
    }
}*/
