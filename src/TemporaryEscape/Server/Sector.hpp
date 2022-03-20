#pragma once

#include "../Scene/Scene.hpp"
#include "../Utils/Worker.hpp"
#include "Messages.hpp"
#include "Services.hpp"
#include "Session.hpp"

namespace wrenbind17 {
class VM;
}

namespace Engine {
class Server;

class Sector : public Scene::EventListener {
public:
    explicit Sector(const Config& config, Services& services, AssetManager& assetManager, TransactionalDatabase& db,
                    std::string galaxyId, std::string systemId, std::string sectorId);
    virtual ~Sector();

    void load();
    void update(float delta);

    void addPlayer(SessionPtr session);

    void eventEntityAdded(const EntityPtr& entity) override;

    void handle(const SessionPtr& session, MessageShipMovement::Request req, MessageShipMovement::Response& res);

private:
    struct PlayerView {
        SessionPtr session;
        std::vector<EntityPtr> entitiesToSync;
    };

    void spawnPlayerShip(SessionPtr session);

    const Config& config;
    Services& services;
    AssetManager& assetManager;
    TransactionalDatabase& db;
    std::string galaxyId;
    std::string systemId;
    std::string sectorId;
    bool loaded;
    Scene scene;
    std::unique_ptr<wrenbind17::VM> vm;

    std::list<PlayerView> players;
    std::vector<Entity::Delta> entityDeltas;

    asio::io_service sync;
};

using SectorPtr = std::shared_ptr<Sector>;
} // namespace Engine
