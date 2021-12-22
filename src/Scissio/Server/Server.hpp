#pragma once

#include "../Assets/AssetManager.hpp"
#include "../Config.hpp"
#include "../Library.hpp"
#include "../Network/NetworkServer.hpp"
#include "../Utils/Worker.hpp"
#include "Database.hpp"
#include "Generator.hpp"
#include "Messages.hpp"
#include "Schemas.hpp"
#include "Sector.hpp"
#include "Session.hpp"

namespace Scissio {
class SCISSIO_API Server {
public:
    explicit Server(const Config& config, AssetManager& assetManager, Database& db, Generator& generator);
    virtual ~Server();

    void tick();

    void eventConnect(const Network::StreamPtr& stream);
    void eventDisconnect(const Network::StreamPtr& stream);
    void eventPacket(const Network::StreamPtr& stream, Network::Packet packet);

    std::vector<SessionPtr> getSessions();

private:
    class EventListener : public Network::EventListener {
    public:
        explicit EventListener(Server& server) : server(server) {
        }

        void eventPacket(const Network::StreamPtr& stream, Network::Packet packet) override {
            server.eventPacket(stream, std::move(packet));
        }

        void eventConnect(const Network::StreamPtr& stream) override {
            server.eventConnect(stream);
        }

        void eventDisconnect(const Network::StreamPtr& stream) override {
            server.eventDisconnect(stream);
        }

    private:
        Server& server;
    };

    struct SectorReference {
        std::string compoundId;
        std::shared_ptr<Sector> ptr;
        bool ready;
    };

    void handle(const Network::StreamPtr& stream, MessageLoginRequest req);
    void handle(const Network::StreamPtr& stream, MessagePingRequest req);
    void handle(const Network::StreamPtr& stream, MessageLatencyRequest req);
    void handle(const Network::StreamPtr& stream, MessageSectorReadyRequest req);
    template <typename T> void handle(const Network::StreamPtr& stream, MessageFetchRequest<T> req);

    template <typename T>
    void fetchResourceForPlayer(const SessionPtr& session, const std::string& prefix, const std::string& start,
                                std::vector<T>& out, std::string& next);

    SessionPtr getSession(const Network::StreamPtr& stream, bool check = true);

    const Config& config;
    AssetManager& assetManager;
    Database& db;
    Generator& generator;

    Network::MessageDispatcher<const Network::StreamPtr&> dispatcher;

    std::thread tickThread;
    std::atomic_bool tickFlag;

    Worker worker;
    Worker::Strand strand;
    BackgroundWorker loader;

    std::unique_ptr<EventListener> listener;
    std::shared_ptr<Network::Server> network;

    std::shared_mutex sessionsMutex;
    std::unordered_map<Network::StreamPtr, SessionPtr> sessions;

    std::vector<SectorReference> sectors;
};
} // namespace Scissio
