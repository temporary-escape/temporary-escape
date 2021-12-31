#include "Server.hpp"
#include "../Network/NetworkTcpServer.hpp"
#include "../Utils/Random.hpp"

#define CMP "Server"

#define DISPATCH_FUNC(M, T, F)                                                                                         \
    std::bind(static_cast<void (T::*)(const Network::StreamPtr&, M)>(&T::F), this, std::placeholders::_1,              \
              std::placeholders::_2)
#define MESSAGE_DISPATCH(M) dispatcher.add<M>(DISPATCH_FUNC(M, Server, handle));
#define MESSAGE_DISPATCH_FETCH(M) dispatcher.add<M>(DISPATCH_FUNC(M, Server, handleFetch));

using namespace Scissio;

static const double tickLength = 1.0f / 20.0f;
static const auto tickLengthUs = std::chrono::microseconds(static_cast<long long>(tickLength * 1000.0 * 1000.0));

Server::Services::Services(const Config& config, AssetManager& assetManager, Database& db)
    : galaxies(config, assetManager, db), regions(config, assetManager, db), systems(config, assetManager, db),
      sectors(config, assetManager, db), players(config, assetManager, db) {
}

Server::Server(const Config& config, AssetManager& assetManager, Database& db)
    : config(config), assetManager(assetManager), db(db), services(config, assetManager, db), worker(4), loader(1),
      listener(std::make_unique<EventListener>(*this)),
      network(std::make_shared<Network::TcpServer>(*listener, config.serverPort)) {

    // tickThread = std::thread(&Server::tick, this);

    MESSAGE_DISPATCH(MessageLoginRequest);
    MESSAGE_DISPATCH(MessageStatusRequest);
    MESSAGE_DISPATCH_FETCH(MessageFetchGalaxySystems);
    MESSAGE_DISPATCH_FETCH(MessageFetchCurrentLocation);

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
    loader.stop();
    worker.stop();
    // tickFlag.store(false);
    // tickThread.join();
}

/*void Server::tick() {
    Log::i(CMP, "Starting tick");

    while (tickFlag.load()) {
        const auto start = std::chrono::steady_clock::now();

        for (const auto& sector : sectors) {
            if (!sector.ready) {
                continue;
            }

            worker.post([=]() {
                try {
                    sector.ptr->update();
                } catch (std::exception& e) {
                    BACKTRACE(CMP, e, "Sector tick failed");
                }
            });
        }

        // This is where the real tick for each sector is performed
        worker.run();

        const auto now = std::chrono::steady_clock::now();
        const auto test = std::chrono::duration_cast<std::chrono::microseconds>(now - start);
        if (test < tickLengthUs) {
            std::this_thread::sleep_for(tickLengthUs - test);
        }
    }

    Log::i(CMP, "Stopped tick");
}*/

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
            auto alreadyLoggedIn = false;
            {
                std::shared_lock<std::shared_mutex> lock{players.mutex};
                for (const auto& [_, other] : players.map) {
                    if (other->getId() == req.secret) {
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

            /*if (!config.serverPassword.empty() && config.serverPassword != req.password) {
                MessageLoginResponse res;
                res.error = "Bad password";
                stream->send(res);
                return;
            }

            // Find the player in the database using its secret
            // to get us a primary key (id)
            const auto found = db.getByIndex<&PlayerData::secret>(req.secret);
            std::string playerId;
            if (found.empty()) {
                // No such player found, generate one
                playerId = uuid();
            } else {
                playerId = found.front().id;
            }

            // Update the player data or create a new player
            db.update<PlayerData>(playerId, [&](std::optional<PlayerData>& player) {
                if (!player.has_value()) {
                    Log::i(CMP, "Registering new player: '{}'", req.name);
                    player = PlayerData{};
                    player.value().id = playerId;
                    player.value().secret = req.secret;
                    player.value().admin = true;
                }

                player.value().name = req.name;

                return true;
            });

            // Update player location
            db.update<PlayerLocationData>(playerId, [&](std::optional<PlayerLocationData>& location) {
                if (!location.has_value()) {
                    Log::i(CMP, "Choosing starting position for player: '{}'", req.name);

                    const auto choices = db.seek<SectorData>("", 1);
                    if (choices.empty()) {
                        EXCEPTION("No choices for starting sector");
                    }
                    const auto& choice = choices.front();

                    location = PlayerLocationData{};
                    location.value().galaxyId = choice.galaxyId;
                    location.value().systemId = choice.systemId;
                    location.value().sectorId = choice.id;

                    return true;
                }

                return false;
            });

            session->id = playerId;

            MessageLoginResponse res;
            res.playerId = playerId;
            stream->send(res);*/

        } catch (std::exception& e) {
            BACKTRACE(CMP, e, "Failed to handle login request");
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

/*void Server::handle(const Network::StreamPtr& stream, MessageSectorReadyRequest req) {
    worker.post([=]() -> void {
        try {
            auto player = streamToPlayer(stream);

            const auto location = db.get<PlayerLocationData>(session->id);
            if (!location) {
                EXCEPTION("Player: '{}' does not have location data", session->id);
            }

            const auto compoundId = fmt::format("{}/{}/{}", location.value().galaxyId, location.value().systemId,
                                                location.value().sectorId);

            // Lookup sector data
            const auto sector = db.get<SectorData>(compoundId);
            if (!sector) {
                EXCEPTION("Player: '{}' is located in non existing sector {}", session->id, compoundId);
            }

            MessageSectorReadyResponse res;

            // Check if we have already initialized this sector
            const auto found = std::find_if(sectors.begin(), sectors.end(),
                                            [&](SectorReference& ref) { return ref.compoundId == compoundId; });

            if (found == sectors.end()) {
                // Sector not found, create it

                Log::i(CMP, "Sector: '{}' is not initialized and is needed by player '{}'", compoundId, session->id);

                sectors.emplace_back();
                auto* sectorPtr = &sectors.back();
                sectorPtr->ptr = std::make_shared<Sector>(*this, db, compoundId);
                sectorPtr->ready = false;
                sectorPtr->compoundId = compoundId;

                // Load the sector
                loader.post([=]() {
                    sectorPtr->ptr->load(generator);
                    sectorPtr->ready = true;
                    Log::i(CMP, "Sector: '{}' is ready", compoundId);
                });

                // Update player location
                session->sectorCompoundId = compoundId;
                sectorPtr->ptr->addSession(session);
            } else {
                res.ready = found->ready;

                // Sector found, update player location
                if (session->sectorCompoundId != found->compoundId) {
                    session->sectorCompoundId = compoundId;
                    found->ptr->addSession(session);
                }
            }

            stream->send(res);

        } catch (std::exception& e) {
            Log::e(CMP, "Failed to handle sector satus request error: {}", e.what());
        }
    });
}*/

template <>
PlayerLocationData Server::fetch<MessageFetchCurrentLocation>(const PlayerPtr& player,
                                                              const MessageFetchCurrentLocation& req,
                                                              std::string& next) {
    return services.players.getLocation(player->getId());
}

template <>
std::vector<SystemData> Server::fetch<MessageFetchGalaxySystems>(const PlayerPtr& player,
                                                                 const MessageFetchGalaxySystems& req,
                                                                 std::string& next) {
    return services.systems.getForPlayer(player->getId(), req.galaxyId, req.token, next);
}

template <typename RequestType> void Server::handleFetch(const Network::StreamPtr& stream, RequestType req) {
    Log::i(CMP, "handleFetch id: {} token: {}", req.id, req.token);

    worker.post([=]() {
        try {
            auto player = streamToPlayer(stream);

            typename RequestType::Response res{};
            res.id = req.id;
            res.data = this->fetch<RequestType>(player, req, res.token);
            stream->send(res);

        } catch (std::exception& e) {
            BACKTRACE(CMP, e,
                      fmt::format("Failed to handle fetch request for type: '{}' id: '{}' token: '{}'",
                                  typeid(RequestType).name(), req.id, req.token));
        }
    });
}
