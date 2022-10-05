#pragma once

#include "../Assets/Registry.hpp"
#include "../Config.hpp"
#include "../Database/Database.hpp"
#include "../Future.hpp"
#include "../Library.hpp"
#include "../Network/NetworkTcpServer.hpp"
#include "../Utils/Worker.hpp"
#include "Generator.hpp"
#include "Messages.hpp"
#include "Schemas.hpp"
#include "Sector.hpp"
#include "Session.hpp"
#include <shared_mutex>

namespace Engine {
class ENGINE_API Server : public NetworkTcpServer<Server, ServerSink> {
public:
    explicit Server(const Config& config, Registry& registry, TransactionalDatabase& db);
    virtual ~Server();

    Future<void> load();
    void tick();

    void onPeerConnected(PeerPtr peer) override;
    void handle(const PeerPtr& peer, MessageLogin::Request req, MessageLogin::Response& res);
    void handle(const PeerPtr& peer, MessageModsInfo::Request req, MessageModsInfo::Response& res);
    void handle(const PeerPtr& peer, MessageRequestSpawn::Request req, MessageRequestSpawn::Response& res);
    void handle(const PeerPtr& peer, MessagePlayerLocation::Request req, MessagePlayerLocation::Response& res);
    void handle(const PeerPtr& peer, MessageFetchGalaxy::Request req, MessageFetchGalaxy::Response& res);
    void handle(const PeerPtr& peer, MessageFetchSystems::Request req, MessageFetchSystems::Response& res);
    void handle(const PeerPtr& peer, MessageFetchRegions::Request req, MessageFetchRegions::Response& res);
    void handle(const PeerPtr& peer, MessageFetchFactions::Request req, MessageFetchFactions::Response& res);
    void handle(const PeerPtr& peer, MessageShipMovement::Request req, MessageShipMovement::Response& res);
    void handle(const PeerPtr& peer, MessageUnlockedBlocks::Request req, MessageUnlockedBlocks::Response& res);

private:
    void addPeerToLobby(const PeerPtr& peer);
    void removePeerFromLobby(const PeerPtr& peer);
    SessionPtr createSession(const PeerPtr& peer, const PlayerData& player);
    SessionPtr peerToSession(const PeerPtr& peer);
    std::tuple<SessionPtr, SectorPtr> peerToSessionSector(const PeerPtr& peer);
    bool isPeerLoggedIn(const std::string& playerId);
    void disconnectPeer(const PeerPtr& peer);
    SectorPtr startSector(const std::string& galaxyId, const std::string& systemId, const std::string& sectorId);
    void addPlayerToSector(const SessionPtr& session, const std::string& sectorId);

    const Config& config;
    Registry& registry;
    TransactionalDatabase& db;
    World world;
    Generator generator;

    std::thread tickThread;
    std::atomic_bool tickFlag;

    Worker::Strand commands;

    // BackgroundWorker& worker;
    // BackgroundWorker loader;

    struct {
        std::shared_mutex mutex;
        std::unordered_set<Peer*> lobby;
        std::unordered_map<Peer*, SessionPtr> sessions;
    } players;

    struct {
        std::shared_mutex mutex;
        std::unordered_map<std::string, std::shared_ptr<Sector>> map;
    } sectors;
};
} // namespace Engine
