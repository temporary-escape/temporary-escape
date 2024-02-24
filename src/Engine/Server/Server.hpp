#pragma once

#include "../Assets/AssetsManager.hpp"
#include "../Config.hpp"
#include "../Database/Database.hpp"
#include "../Future.hpp"
#include "../Library.hpp"
#include "../Network/NetworkDispatcher.hpp"
#include "../Utils/PerformanceRecord.hpp"
#include "../Utils/Worker.hpp"
#include "Generator.hpp"
#include "Messages.hpp"
#include "PeerLobby.hpp"
#include "PlayerSessions.hpp"
#include "Sector.hpp"
#include "Service.hpp"
#include <shared_mutex>

namespace Engine {
class ENGINE_API Matchmaker;
class ENGINE_API Lua;
class ENGINE_API NetworkUdpServer;

class ENGINE_API Server : public NetworkDispatcher2 {
public:
    explicit Server(const Config& config, AssetsManager& assetsManager, Database& db);
    virtual ~Server();

    void onAcceptSuccess(const NetworkStreamPtr& peer) override;
    void onDisconnect(const NetworkStreamPtr& peer) override;

    uint16_t getPort() const {
        return port;
    }

    // Player specific requests
    void handle(Request2<MessageLoginRequest> req);
    void handle(Request2<MessagePlayerSpawnRequest> req);
    void handle(Request2<MessageModManifestsRequest> req);
    void handle(Request2<MessagePingResponse> req);
    void handle(Request2<MessageActionApproach> req);
    void handle(Request2<MessageActionOrbit> req);
    void handle(Request2<MessageActionKeepDistance> req);
    void handle(Request2<MessageActionStopMovement> req);
    void handle(Request2<MessageControlTargetEvent> req);

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

    Generator& getGenerator() {
        return *generator;
    }

    // Sector functions
    void movePlayerToSector(const std::string& playerId, const std::string& sectorId);
    SessionPtr getPlayerSession(const std::string& playerId);
    SectorPtr startSector(const std::string& sectorId);
    void addPlayerToSector(const SessionPtr& session, const std::string& sectorId);
    SectorPtr getSectorForSession(const SessionPtr& session);
    template <typename T> void forwardMessageToSector(const Request2<T>& req);

    static Server* instance;

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

    const Config& config;
    AssetsManager& assetsManager;
    Database& db;
    PlayerSessions playerSessions;
    PeerLobby lobby;
    std::unique_ptr<EventBus> eventBus;
    std::unordered_map<uint64_t, std::unique_ptr<Service>> services;

    std::thread tickThread;
    std::atomic_bool tickFlag;

    std::unique_ptr<Generator> generator;
    std::unique_ptr<Lua> lua;
    BackgroundWorker worker;
    BackgroundWorker loadQueue;
    Worker::Strand strand;
    std::shared_ptr<NetworkUdpServer> network;
    uint16_t port;

    struct {
        std::shared_mutex mutex;
        std::unordered_map<std::string, std::shared_ptr<Sector>> map;
    } sectors;

    struct {
        PerformanceRecord tickTime;
    } perf;
};
} // namespace Engine
