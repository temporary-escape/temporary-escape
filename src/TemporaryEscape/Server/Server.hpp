#pragma once

#include "../Assets/AssetManager.hpp"
#include "../Config.hpp"
#include "../Future.hpp"
#include "../Library.hpp"
#include "../Modding/ModManager.hpp"
#include "../Network/NetworkTcpServer.hpp"
#include "../Utils/Database.hpp"
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
    explicit Server(const Config& config, ModManager& modManager, AssetManager& assetManager,
                    TransactionalDatabase& db);
    virtual ~Server();

    void load();
    void tick();

    void onPeerConnected(PeerPtr peer) override;
    void handle(const PeerPtr& peer, MessageLogin::Request req, MessageLogin::Response& res);
    void handle(const PeerPtr& peer, MessageModsInfo::Request req, MessageModsInfo::Response& res);
    void handle(const PeerPtr& peer, MessagePlayerLocation::Request req, MessagePlayerLocation::Response& res);
    void handle(const PeerPtr& peer, MessageFetchGalaxy::Request req, MessageFetchGalaxy::Response& res);
    void handle(const PeerPtr& peer, MessageFetchSystems::Request req, MessageFetchSystems::Response& res);
    void handle(const PeerPtr& peer, MessageFetchRegions::Request req, MessageFetchRegions::Response& res);
    void handle(const PeerPtr& peer, MessageFetchFactions::Request req, MessageFetchFactions::Response& res);
    void handle(const PeerPtr& peer, MessageShipMovement::Request req, MessageShipMovement::Response& res);
    void handle(const PeerPtr& peer, MessageUnlockedBlocks::Request req, MessageUnlockedBlocks::Response& res);

private:
    SessionPtr peerToSession(const PeerPtr& peer);
    std::tuple<SessionPtr, SectorPtr> peerToSessionSector(const PeerPtr& peer);

    const Config& config;
    ModManager& modManager;
    AssetManager& assetManager;
    TransactionalDatabase& db;
    World world;
    Generator generator;
    Promise<void> loaded;

    std::thread tickThread;
    std::atomic_bool tickFlag;

    BackgroundWorker& worker;
    BackgroundWorker loader;

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
