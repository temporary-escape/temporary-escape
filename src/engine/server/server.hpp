#pragma once

#include "../assets/assets_manager.hpp"
#include "../config.hpp"
#include "../database/database.hpp"
#include "../future.hpp"
#include "../library.hpp"
#include "../network/network_dispatcher.hpp"
#include "../utils/performance_record.hpp"
#include "../utils/worker.hpp"
#include "generator.hpp"
#include "messages.hpp"
#include "peer_lobby.hpp"
#include "player_sessions.hpp"
#include "sector.hpp"
#include <shared_mutex>

namespace Engine {
class ENGINE_API Lua;
class ENGINE_API NetworkTcpServer;

class ENGINE_API Server : public NetworkDispatcher {
public:
    explicit Server(const Config& config, AssetsManager& assetsManager, Database& db);
    virtual ~Server();

    void onAcceptSuccess(const NetworkPeerPtr& peer) override;
    void onDisconnect(const NetworkPeerPtr& peer) override;

    // Player specific requests
    void handle(Request<MessageLoginRequest> req);
    void handle(Request<MessageModManifestsRequest> req);
    void handle(Request<MessagePlayerLocationRequest> req);
    void handle(Request<MessagePingResponse> req);

    EventBus& getEventBus() const;
    AssetsManager& getAssetManager() const {
        return assetsManager;
    }
    Database& getDatabase() {
        return db;
    }
    PlayerSessions& getPlayerSessions() {
        return playerSessions;
    }
    PeerLobby& getPeerLobby() {
        return lobby;
    }
    std::chrono::nanoseconds getPerfTickTime() const {
        return perf.tickTime.value();
    }
    void disconnectPlayer(const std::string& playerId);

    static Server* instance;

    static void bind(Lua& lua);

private:
    void load();
    void tick();
    void cleanup();
    void pollEvents();
    void updateSectors();

    // Sector functions
    void movePlayerToSector(const std::string& playerId, const std::string& sectorId);
    SessionPtr getPlayerSession(const std::string& playerId);
    SectorPtr startSector(const std::string& galaxyId, const std::string& systemId, const std::string& sectorId);
    void addPlayerToSector(const SessionPtr& session, const std::string& sectorId);

private:
    const Config& config;
    AssetsManager& assetsManager;
    Database& db;
    Generator generator;
    PlayerSessions playerSessions;
    PeerLobby lobby;
    World world;
    std::unique_ptr<EventBus> eventBus;

    std::thread tickThread;
    std::atomic_bool tickFlag;

    std::unique_ptr<Lua> lua;
    BackgroundWorker worker;
    BackgroundWorker loadQueue;
    std::unique_ptr<NetworkTcpServer> networkServer;

    struct {
        std::shared_mutex mutex;
        std::unordered_map<std::string, std::shared_ptr<Sector>> map;
    } sectors;

    struct {
        PerformanceRecord tickTime;
    } perf;
};
} // namespace Engine
