#pragma once

#include "../assets/assets_manager.hpp"
#include "../config.hpp"
#include "../database/database.hpp"
#include "../future.hpp"
#include "../library.hpp"
#include "../network/server.hpp"
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

class ENGINE_API Server : public Network::Server {
public:
    struct Certs {
        Certs() : key{}, cert{key}, dh{} {
        }

        Network::Pkey key;
        Network::Cert cert;
        Network::Dh dh;
    };

    explicit Server(const Config& config, const Certs& certs, AssetsManager& assetsManager, Database& db);
    virtual ~Server();

    void onAcceptSuccess(PeerPtr peer) override;

    // Player specific requests
    void handle(const PeerPtr& peer, MessageLoginRequest req, MessageLoginResponse& res);
    void handle(const PeerPtr& peer, MessageModManifestsRequest req, MessageModManifestsResponse& res);
    void handle(const PeerPtr& peer, MessagePlayerLocationRequest req, MessagePlayerLocationResponse& res);
    void handle(const PeerPtr& peer, MessagePingResponse res);

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

    // Network overrides
    void onError(std::error_code ec) override;
    void onError(const PeerPtr& peer, std::error_code ec) override;
    void onUnhandledException(const PeerPtr& peer, std::exception_ptr& eptr) override;
    void postDispatch(std::function<void()> fn) override;

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

    struct {
        std::shared_mutex mutex;
        std::unordered_map<std::string, std::shared_ptr<Sector>> map;
    } sectors;

    struct {
        PerformanceRecord tickTime;
    } perf;
};
} // namespace Engine
