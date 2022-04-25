#pragma once

#include "../Assets/AssetManager.hpp"
#include "../Config.hpp"
#include "../Future.hpp"
#include "../Library.hpp"
#include "../Network/NetworkTcpServer.hpp"
#include "../Utils/Database.hpp"
#include "../Utils/Worker.hpp"
#include "Messages.hpp"
#include "Schemas.hpp"
#include "Sector.hpp"
#include "SectorLoader.hpp"
#include "Services.hpp"
#include "Session.hpp"
#include <shared_mutex>

namespace Engine {
class ENGINE_API Server : public NetworkTcpServer<Server, ServerSink> {
public:
    explicit Server(const Config& config, AssetManager& assetManager, TransactionalDatabase& db);
    virtual ~Server();

    void load();
    void tick();

    /*void eventConnect(const Network::StreamPtr& stream);
    void eventDisconnect(const Network::StreamPtr& stream);
    void eventPacket(const Network::StreamPtr& stream, Network::Packet packet);*/

    Services& getServices() {
        return services;
    }

    void onPeerConnected(PeerPtr peer) override;
    void handle(const PeerPtr& peer, MessageLogin::Request req, MessageLogin::Response& res);
    void handle(const PeerPtr& peer, MessageFetchGalaxy::Request req, MessageFetchGalaxy::Response& res);
    void handle(const PeerPtr& peer, MessageFetchSystems::Request req, MessageFetchSystems::Response& res);
    void handle(const PeerPtr& peer, MessageFetchRegions::Request req, MessageFetchRegions::Response& res);
    void handle(const PeerPtr& peer, MessageShipMovement::Request req, MessageShipMovement::Response& res);

private:
    /*class EventListener : public Network::EventListener {
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
    };*/

    /*void handle(const Network::StreamPtr& stream, MessageLoginRequest req);
    void handle(const Network::StreamPtr& stream, MessageStatusRequest req);
    template <typename RequestType> void handleFetch(const Network::StreamPtr& stream, RequestType req);*/

    /*template <typename RequestType>
    typename RequestType::Response::ItemType fetch(const PlayerPtr& player, const RequestType& req, std::string&
    next);*/

    /*PlayerPtr streamToPlayer(const Network::StreamPtr& stream, bool check = true);*/
    // SectorPtr startSector(const std::string& galaxyId, const std::string& systemId, const std::string& sectorId);

    SessionPtr peerToSession(const PeerPtr& peer);
    std::tuple<SessionPtr, SectorPtr> peerToSessionSector(const PeerPtr& peer);

    const Config& config;
    AssetManager& assetManager;
    TransactionalDatabase& db;
    Services services;
    Promise<void> loaded;

    // Network::MessageDispatcher<const Network::StreamPtr&> dispatcher;

    std::thread tickThread;
    std::atomic_bool tickFlag;

    // Worker worker;
    // Worker::Strand strand;
    BackgroundWorker& worker;
    BackgroundWorker loader;

    // std::unique_ptr<EventListener> listener;
    // std::shared_ptr<Network::Server> network;

    struct PlayersInternal {
        std::shared_mutex mutex;
        std::unordered_set<Peer*> lobby;
        std::unordered_map<Peer*, SessionPtr> map;
        std::unordered_map<Session*, std::string> sectors;
    } players;

    struct SectorsInternal {
        std::shared_mutex mutex;
        std::unordered_map<std::string, std::shared_ptr<Sector>> map;
    } sectors;
};
} // namespace Engine
