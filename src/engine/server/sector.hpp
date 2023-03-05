#pragma once

#include "../scene/scene.hpp"
#include "../utils/worker.hpp"
#include "messages.hpp"
#include "session.hpp"
#include "world.hpp"

namespace Engine {
class Server;

class Sector : public Scene::EventListener {
public:
    explicit Sector(const Config& config, World& world, Registry& registry, EventBus& eventBus, std::string galaxyId,
                    std::string systemId, std::string sectorId);
    virtual ~Sector();

    void load();
    void update(float delta);

    void addPlayer(SessionPtr session);

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
    struct PlayerData {
        SessionPtr session;
    };

    void spawnPlayerShip(SessionPtr session);

    const Config& config;
    World& world;
    Registry& registry;
    EventBus& eventBus;
    std::string galaxyId;
    std::string systemId;
    std::string sectorId;
    bool loaded;
    Scene scene;
    // std::unique_ptr<wrenbind17::VM> vm;

    std::list<PlayerData> players;

    asio::io_service sync;
};

using SectorPtr = std::shared_ptr<Sector>;
} // namespace Engine
