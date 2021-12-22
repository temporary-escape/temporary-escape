#include "Server.hpp"
#include "../Network/NetworkTcpServer.hpp"
#include "../Utils/Random.hpp"

#define CMP "Server"

#define DISPATCH_FUNC(M, T, F)                                                                                         \
    std::bind(static_cast<void (T::*)(const Network::StreamPtr&, M)>(&T::F), this, std::placeholders::_1,              \
              std::placeholders::_2)
#define MESSAGE_DISPATCH(M) dispatcher.add<M>(DISPATCH_FUNC(M, Server, handle));

using namespace Scissio;

static const double tickLength = 1.0f / 20.0f;
static const auto tickLengthUs = std::chrono::microseconds(static_cast<long long>(tickLength * 1000.0 * 1000.0));

Server::Server(const Config& config, AssetManager& assetManager, Database& db, Generator& generator)
    : config(config), tickFlag(true), assetManager(assetManager), db(db), generator(generator), worker(4),
      strand(worker.strand()), listener(std::make_unique<EventListener>(*this)),
      network(std::make_shared<Network::TcpServer>(*listener, config.serverPort)) {

    tickThread = std::thread(&Server::tick, this);

    MESSAGE_DISPATCH(MessageLoginRequest);
    MESSAGE_DISPATCH(MessagePingRequest);
    MESSAGE_DISPATCH(MessageLatencyRequest);
    MESSAGE_DISPATCH(MessageSectorReadyRequest);
    MESSAGE_DISPATCH(MessageFetchRequest<SystemData>);
}

Server::~Server() {
    Log::i(CMP, "Stopping tick");
    tickFlag.store(false);
    tickThread.join();
}

void Server::tick() {
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
}

void Server::eventConnect(const Network::StreamPtr& stream) {
    strand.post([=]() {
        try {
            Log::i(CMP, "Adding new connection to lobby: {:#0x}", reinterpret_cast<uint64_t>(stream.get()));
            std::unique_lock<std::shared_mutex> lock{sessionsMutex};
            auto session = std::make_shared<Session>();
            session->stream = stream;
            sessions.insert(std::make_pair(stream, std::move(session)));
        } catch (std::exception& e) {
            Log::e(CMP, "Failed to handle connect event error: {}", e.what());
        }
    });
}

void Server::eventDisconnect(const Network::StreamPtr& stream) {
    strand.post([=]() {
        try {
            Log::i(CMP, "Removing connection: {:#0x}", reinterpret_cast<uint64_t>(stream.get()));
            std::unique_lock<std::shared_mutex> lock{sessionsMutex};
            sessions.erase(stream);
        } catch (std::exception& e) {
            Log::e(CMP, "Failed to handle connect event error: {}", e.what());
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

SessionPtr Server::getSession(const Network::StreamPtr& stream, bool check) {
    std::shared_lock<std::shared_mutex> lock{sessionsMutex};
    const auto it = sessions.find(stream);
    if (it != sessions.end()) {
        if (check && it->second->playerId.empty()) {
            EXCEPTION("Session does is not logged in for stream: {:#0x}", reinterpret_cast<uint64_t>(stream.get()));
        }
        return it->second;
    }
    EXCEPTION("Session does not exist for stream: {:#0x}", reinterpret_cast<uint64_t>(stream.get()));
}

std::vector<SessionPtr> Server::getSessions() {
    std::shared_lock<std::shared_mutex> lock{sessionsMutex};
    std::vector<SessionPtr> res;
    for (const auto& [stream, session] : sessions) {
        res.push_back(session);
    }
    return res;
}

template <typename T>
void Server::fetchResourceForPlayer(const SessionPtr& session, const std::string& prefix, const std::string& start,
                                    std::vector<T>& out, std::string& next) {

    out = db.next<T>(prefix, start, 64, &next);
}

void Server::handle(const Network::StreamPtr& stream, MessageLoginRequest req) {
    strand.post([=]() -> void {
        try {
            Log::i(CMP, "Logging in player: '{}'", req.name);
            auto session = getSession(stream, false);

            if (!config.serverPassword.empty() && config.serverPassword != req.password) {
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

            session->playerId = playerId;

            MessageLoginResponse res;
            res.playerId = playerId;
            stream->send(res);

        } catch (std::exception& e) {
            Log::e(CMP, "Failed to handle login request error: {}", e.what());
        }
    });
}

void Server::handle(const Network::StreamPtr& stream, MessagePingRequest req) {
    MessagePingResponse res;
    res.timePoint = req.timePoint;
    stream->send(res);
}

void Server::handle(const Network::StreamPtr& stream, MessageLatencyRequest req) {
    worker.post([=]() {
        MessageLatencyResponse res;
        res.timePoint = req.timePoint;
        stream->send(res);
    });
}

void Server::handle(const Network::StreamPtr& stream, MessageSectorReadyRequest req) {
    strand.post([=]() -> void {
        try {
            auto session = getSession(stream);

            const auto location = db.get<PlayerLocationData>(session->playerId);
            if (!location) {
                EXCEPTION("Player: '{}' does not have location data", session->playerId);
            }

            const auto compoundId = fmt::format("{}/{}/{}", location.value().galaxyId, location.value().systemId,
                                                location.value().sectorId);

            // Lookup sector data
            const auto sector = db.get<SectorData>(compoundId);
            if (!sector) {
                EXCEPTION("Player: '{}' is located in non existing sector {}", session->playerId, compoundId);
            }

            MessageSectorReadyResponse res;

            // Check if we have already initialized this sector
            const auto found = std::find_if(sectors.begin(), sectors.end(),
                                            [&](SectorReference& ref) { return ref.compoundId == compoundId; });

            if (found == sectors.end()) {
                // Sector not found, create it

                Log::i(CMP, "Sector: '{}' is not initialized and is needed by player '{}'", compoundId,
                       session->playerId);

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
}

template <typename T> void Server::handle(const Network::StreamPtr& stream, MessageFetchRequest<T> req) {
    worker.post([=]() {
        try {
            auto session = getSession(stream);

            // Log::d(CMP, "Handle MessageFetchRequest<{}> start: {}", typeid(T).name(), req.start);

            MessageFetchResponse<T> res;
            res.id = req.id;
            res.prefix = req.prefix;
            try {
                fetchResourceForPlayer(session, req.prefix, req.start, res.data, res.next);
            } catch (std::exception& e) {
                Log::e(CMP, "Failed to fetch resource for: '{}' player: '{}' error: {}", typeid(T).name(),
                       session->playerId, e.what());
                res.data.clear();
                res.next.clear();
                res.error = "Failed to fetch resource";
            }
            stream->send(res);

        } catch (std::exception& e) {
            Log::e(CMP, "Failed to handle fetch request for: '{}' next: '{}' start: '{}'", typeid(T).name(), req.start,
                   e.what());
        }
    });
}
