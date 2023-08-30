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
#include "service.hpp"
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
    void handle(Request<MessagePlayerSpawnRequest> req);
    void handle(Request<MessageModManifestsRequest> req);
    void handle(Request<MessagePingResponse> req);
    void handle(Request<MessageControlMovementEvent> req);
    void handle(Request<MessageControlTargetEvent> req);

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

    template <typename T> T& getService() {
        const auto it = services.find(typeid(T).hash_code());
        if (it == services.end()) {
            EXCEPTION("No such service: {}", typeid(T).name());
        }
        return *dynamic_cast<T*>(it->second.get());
    }

    void addSectorType(std::string name, SectorType type) {
        generator.addSectorType(std::move(name), std::move(type));
    }

    static Server* instance;

    static void bind(Lua& lua);

private:
    void load();
    void startTick();
    void tick();
    void cleanup();
    void pollEvents();
    void updateSectors();
    template <typename T, typename... Args> void addService(Args&&... args) {
        services.emplace(typeid(T).hash_code(),
                         std::make_unique<T>(*this, db, playerSessions, std::forward<Args>(args)...));
    }

    // Sector functions
    void movePlayerToSector(const std::string& playerId, const std::string& sectorId);
    SessionPtr getPlayerSession(const std::string& playerId);
    SectorPtr startSector(const std::string& sectorId);
    void addPlayerToSector(const SessionPtr& session, const std::string& sectorId);
    SectorPtr getSectorForSession(const SessionPtr& session);

    const Config& config;
    AssetsManager& assetsManager;
    Database& db;
    Generator generator;
    PlayerSessions playerSessions;
    PeerLobby lobby;
    std::unique_ptr<EventBus> eventBus;
    std::unordered_map<uint64_t, std::unique_ptr<Service>> services;

    std::thread tickThread;
    std::atomic_bool tickFlag;

    std::unique_ptr<Lua> lua;
    BackgroundWorker worker;
    BackgroundWorker loadQueue;
    Worker::Strand strand;
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
