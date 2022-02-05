#include "Server.hpp"
#include "../Network/NetworkTcpServer.hpp"
#include "../Utils/Random.hpp"

#define CMP "Server"

#define DISPATCH_FUNC(M, T, F)                                                                                         \
    std::bind(static_cast<void (T::*)(const Network::StreamPtr&, M)>(&T::F), this, std::placeholders::_1,              \
              std::placeholders::_2)
#define MESSAGE_DISPATCH(M) dispatcher.add<M>(DISPATCH_FUNC(M, Server, handle));
#define MESSAGE_DISPATCH_FETCH(M) dispatcher.add<M>(DISPATCH_FUNC(M, Server, handleFetch));

using namespace Engine;

Server::Server(const Config& config, AssetManager& assetManager, Database& db)
    : config(config), assetManager(assetManager), db(db), services(config, assetManager, db),
      sectorLoader(config, assetManager, db, services), tickFlag(true), worker(4), loader(1),
      listener(std::make_unique<EventListener>(*this)),
      network(std::make_shared<Network::TcpServer>(*listener, config.serverPort)) {

    tickThread = std::thread(&Server::tick, this);

    MESSAGE_DISPATCH(MessageLoginRequest);
    MESSAGE_DISPATCH(MessageStatusRequest);
    MESSAGE_DISPATCH_FETCH(MessageFetchGalaxy);
    MESSAGE_DISPATCH_FETCH(MessageFetchGalaxySystems);
    MESSAGE_DISPATCH_FETCH(MessageFetchGalaxyRegions);
    MESSAGE_DISPATCH_FETCH(MessageFetchSystemPlanets);
    MESSAGE_DISPATCH_FETCH(MessageFetchCurrentLocation);
}

void Server::load() {
    loader.post([this]() {
        try {
            services.galaxies.generate(123456789ULL);
            services.regions.generate();
            services.systems.generate();
            services.sectors.generate();
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
                sectorWorker.post([&failedId, sector, compoundId]() {
                    try {
                        sector->update();
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

void Server::eventConnect(const Network::StreamPtr& stream) {
    worker.post([=]() {
        try {
            Log::i(CMP, "Adding new connection to lobby: {:#0x}", reinterpret_cast<uint64_t>(stream.get()));
            std::unique_lock<std::shared_mutex> lock{players.mutex};
            auto player = std::make_shared<Player>(stream);
            players.map.insert(std::make_pair(stream, player));

            MessageServerHello msg{};
            msg.serverName = "Some Server Name";
            player->send(msg);
        } catch (std::exception& e) {
            BACKTRACE(CMP, e, "Failed to handle connect event");
        }
    });
}

void Server::eventDisconnect(const Network::StreamPtr& stream) {
    worker.post([=]() {
        try {
            Log::i(CMP, "Removing connection: {:#0x}", reinterpret_cast<uint64_t>(stream.get()));
            std::unique_lock<std::shared_mutex> lock{players.mutex};
            players.map.erase(stream);
        } catch (std::exception& e) {
            BACKTRACE(CMP, e, "Failed to handle disconnect event");
        }
    });
}

void Server::eventPacket(const Network::StreamPtr& stream, Network::Packet packet) {
    try {
        dispatcher.dispatch(stream, packet);
    } catch (...) {
        EXCEPTION_NESTED("Failed to dispatch message");
    }
}

PlayerPtr Server::streamToPlayer(const Network::StreamPtr& stream, bool check) {
    std::shared_lock<std::shared_mutex> lock{players.mutex};
    const auto it = players.map.find(stream);
    if (it != players.map.end()) {
        if (check && it->second->getId().empty()) {
            EXCEPTION("Session does is not logged in for stream: {:#0x}", reinterpret_cast<uint64_t>(stream.get()));
        }
        return it->second;
    }
    EXCEPTION("Session does not exist for stream: {:#0x}", reinterpret_cast<uint64_t>(stream.get()));
}

SectorPtr Server::startSector(const std::string& galaxyId, const std::string& systemId, const std::string& sectorId) {
    std::shared_lock<std::shared_mutex> lock{sectors.mutex};
    const auto compoundId = fmt::format("{}/{}/{}", galaxyId, systemId, sectorId);

    const auto test = db.get<SectorData>(compoundId);
    if (!test) {
        EXCEPTION("Sector: '{}' does not exist", compoundId);
    }

    auto it = sectors.map.find(compoundId);
    if (it == sectors.map.end()) {
        const auto sector = std::make_shared<Sector>(*this, db, compoundId);
        it = sectors.map.insert(std::make_pair(compoundId, sector)).first;
    }

    return it->second;
}

std::vector<PlayerPtr> Server::getPlayers() {
    std::shared_lock<std::shared_mutex> lock{players.mutex};
    std::vector<PlayerPtr> res;
    for (const auto& [stream, player] : players.map) {
        res.push_back(player);
    }
    return res;
}

/*template <typename T>
void Server::fetchResourceForPlayer(const SessionPtr& session, const std::string& prefix, const std::string& start,
                                    std::vector<T>& out, std::string& next) {

    out = db.next<T>(prefix, start, 64, &next);
}*/

void Server::handle(const Network::StreamPtr& stream, MessageLoginRequest req) {
    worker.post([=]() -> void {
        try {
            Log::i(CMP, "Logging in player: '{}'", req.name);
            auto player = streamToPlayer(stream, false);

            // Check for server password
            if (!config.serverPassword.empty() && config.serverPassword != req.password) {
                MessageLoginResponse res;
                res.error = "Bad password";
                stream->send(res);
                return;
            }

            // Check if the player is already logged in
            const auto playerIdFound = services.players.secretToId(req.secret);
            if (playerIdFound) {
                auto alreadyLoggedIn = false;
                {
                    std::shared_lock<std::shared_mutex> lock{players.mutex};
                    for (const auto& [_, other] : players.map) {
                        if (other->getId() == playerIdFound.value()) {
                            alreadyLoggedIn = true;
                            break;
                        }
                    }
                }

                if (alreadyLoggedIn) {
                    MessageLoginResponse res;
                    res.error = "Already logged in";
                    stream->send(res);
                    return;
                }
            }

            // Login player, this will either update the existing record or create a new one
            const auto data = services.players.login(req.secret, req.name);
            player->setId(data.id);

            // Find starting location so we can move the player to the correct sector
            const auto location = services.players.findStartingLocation(data);

            const auto sector = startSector(location.galaxyId, location.systemId, location.sectorId);

            // Send login response
            MessageLoginResponse res;
            res.playerId = data.id;
            stream->send(res);

            // Load the sector if needed and add the player to that sector.
            loader.post([=]() {
                try {
                    sector->load();
                    sector->addPlayer(player);
                } catch (std::exception& e) {
                    BACKTRACE(CMP, e, "Failed to load sector id: '{}'", sector->getCompoundId());
                }
            });

        } catch (std::exception& e) {
            BACKTRACE(CMP, e, "Failed to handle login request for player: '{}'", req.name);
        }
    });
}

void Server::handle(const Network::StreamPtr& stream, MessageStatusRequest req) {
    worker.post([=]() {
        MessageStatusResponse res;
        res.timePoint = req.timePoint;
        stream->send(res);
    });
}

template <>
PlayerLocationData Server::fetch<MessageFetchCurrentLocation>(const PlayerPtr& player,
                                                              const MessageFetchCurrentLocation& req,
                                                              std::string& next) {
    return services.players.getLocation(player->getId());
}

template <>
GalaxyData Server::fetch<MessageFetchGalaxy>(const PlayerPtr& player, const MessageFetchGalaxy& req,
                                             std::string& next) {
    return services.galaxies.getForPlayer(player->getId(), req.galaxyId);
}

template <>
std::vector<SystemData> Server::fetch<MessageFetchGalaxySystems>(const PlayerPtr& player,
                                                                 const MessageFetchGalaxySystems& req,
                                                                 std::string& next) {
    return services.systems.getForPlayer(player->getId(), req.galaxyId, req.token, next);
}

template <>
std::vector<RegionData> Server::fetch<MessageFetchGalaxyRegions>(const PlayerPtr& player,
                                                                 const MessageFetchGalaxyRegions& req,
                                                                 std::string& next) {
    return services.regions.getForPlayer(player->getId(), req.galaxyId, req.token, next);
}

template <>
std::vector<SectorPlanetData> Server::fetch<MessageFetchSystemPlanets>(const PlayerPtr& player,
                                                                       const MessageFetchSystemPlanets& req,
                                                                       std::string& next) {
    return services.systems.getSystemPlanets(req.galaxyId, req.systemId);
}

template <typename RequestType> void Server::handleFetch(const Network::StreamPtr& stream, RequestType req) {
    worker.post([=]() {
        try {
            Log::d(CMP, "handle fetch for type: '{}' id: '{}' token: '{}'", typeid(RequestType).name(), req.id,
                   req.token);
            auto player = streamToPlayer(stream);

            typename RequestType::Response res{};
            res.id = req.id;
            res.data = this->fetch<RequestType>(player, req, res.token);
            stream->send(res);

        } catch (std::exception& e) {
            BACKTRACE(CMP, e, "Failed to handle fetch request for type: '{}' id: '{}' token: '{}'",
                      typeid(RequestType).name(), req.id, req.token);
        }
    });
}
