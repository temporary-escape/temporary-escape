#pragma once

#include "../scene/scene.hpp"
#include "../utils/worker.hpp"
#include "lua.hpp"
#include "messages.hpp"
#include "session.hpp"
#include "world.hpp"

namespace Engine {
class ENGINE_API Server;

class ENGINE_API Sector {
public:
    explicit Sector(const Config& config, Database& db, AssetsManager& assetsManager, EventBus& eventBus,
                    std::string galaxyId, std::string systemId, std::string sectorId);
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

    // void handle(const SessionPtr& session, MessageShipMovement::Request req, MessageShipMovement::Response& res);

private:
    const Config& config;
    Database& db;
    AssetsManager& assetsManager;
    EventBus& eventBus;
    uint64_t tickCount{0};
    std::string galaxyId;
    std::string systemId;
    std::string sectorId;
    bool loaded;
    std::unique_ptr<Scene> scene;
    std::unique_ptr<Lua> lua;
    std::vector<SessionPtr> players;
    SynchronizedWorker worker;
};

using SectorPtr = std::shared_ptr<Sector>;
} // namespace Engine
