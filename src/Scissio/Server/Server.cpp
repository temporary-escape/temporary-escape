#include "Server.hpp"

#define CMP "Server"

#define DISPATCH_FUNC(M, T, F)                                                                                         \
    std::bind(static_cast<void (T::*)(const Network::StreamPtr&, M)>(&T::F), this, std::placeholders::_1,              \
              std::placeholders::_2)
#define MESSAGE_DISPATCH(M) dispatcher.add<M>(DISPATCH_FUNC(M, Server, handle));

using namespace Scissio;

static const double tickLength = 1.0f / 20.0f;
static const auto tickLengthUs = std::chrono::microseconds(static_cast<long long>(tickLength * 1000.0 * 1000.0));

Server::Server(const Config& config, AssetManager& assetManager, Database& db)
    : Network::TcpServer(config.serverPort), config(config), assetManager(assetManager), db(db), worker(4),
      strand(worker.strand()) {

    tickFlag.store(true);
    tickThread = std::thread(&Server::tick, this);

    MESSAGE_DISPATCH(MessageLoginRequest);
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

        // This is where the real tick for each zone is performed
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

SessionPtr Server::getSession(const Network::StreamPtr& stream) {
    std::shared_lock<std::shared_mutex> lock{sessionsMutex};
    const auto it = sessions.find(stream);
    if (it != sessions.end()) {
        return it->second;
    }
    EXCEPTION("Session does not exist for stream: {:#0x}", reinterpret_cast<uint64_t>(stream.get()));
}

void Server::handle(const Network::StreamPtr& stream, MessageLoginRequest req) {
    strand.post([=]() {
        try {
            Log::i(CMP, "Logging in player uid: {}", req.uid);
            auto session = getSession(stream);

            const auto key = fmt::format("player_{}", req.uid);
            auto data = db.get<PlayerData>(key);

            if (!data.has_value()) {
                PlayerData player;
                player.uid = req.uid;
                player.name = req.name;
                player.admin = true;
                db.put(key, player);
            }

            session->uid = req.uid;
        } catch (std::exception& e) {
            Log::e(CMP, "Failed to handle login request error: {}", e.what());
        }
    });
}
