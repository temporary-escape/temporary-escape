#pragma once

#include "../Scene/Scene.hpp"
#include "../Utils/Worker.hpp"
#include "Generator.hpp"
#include "Lua.hpp"
#include "Messages.hpp"
#include "Session.hpp"

namespace Engine {
class ENGINE_API Server;

class ENGINE_API Sector {
public:
    explicit Sector(const Config& config, Database& db, AssetsManager& assetsManager, EventBus& eventBus,
                    Generator& generator, std::string galaxyId, std::string systemId, std::string sectorId);
    virtual ~Sector();

    void load();
    void update();

    void addPlayer(const SessionPtr& session);
    void removePlayer(const SessionPtr& session);

    bool isLoaded() const {
        return loaded;
    }

    const std::string& getGalaxyId() const {
        return galaxyId;
    }

    const std::string& getSystemId() const {
        return systemId;
    }

    const std::string& getSectorId() const {
        return sectorId;
    }

    void handle(const SessionPtr& session, MessageControlMovementEvent req);
    void handle(const SessionPtr& session, MessageControlTargetEvent req);

    // void handle(const SessionPtr& session, MessageShipMovement::Request req, MessageShipMovement::Response& res);

private:
    Entity spawnPlayerEntity(const SessionPtr& session);

    const Config& config;
    Database& db;
    AssetsManager& assetsManager;
    EventBus& eventBus;
    Generator& generator;
    uint64_t tickCount{0};
    std::string galaxyId;
    std::string systemId;
    std::string sectorId;
    bool loaded;
    std::unique_ptr<Scene> scene;
    std::unique_ptr<Lua> lua;
    std::vector<SessionPtr> players;
    std::unordered_map<SessionPtr, entt::entity> playerControl;
    SynchronizedWorker worker;
};

using SectorPtr = std::shared_ptr<Sector>;
} // namespace Engine
