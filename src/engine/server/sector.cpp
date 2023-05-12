#include "sector.hpp"
#include "../utils/string_utils.hpp"
#include "server.hpp"

using namespace Engine;

static auto logger = createLogger(__FILENAME__);

Sector::Sector(const Config& config, World& world, Registry& registry, EventBus& eventBus, std::string galaxyId,
               std::string systemId, std::string sectorId) :
    config{config},
    world{world},
    registry{registry},
    eventBus{eventBus},
    galaxyId{std::move(galaxyId)},
    systemId{std::move(systemId)},
    sectorId{std::move(sectorId)},
    loaded{false} {

    logger.info("Started sector: '{}'", this->sectorId);
}

Sector::~Sector() {
    logger.info("Stopped sector: '{}'", sectorId);
}

void Sector::load() {
    if (loaded) {
        return;
    }

    loaded = true;

    try {
        auto found = world.sectors.find(galaxyId, systemId, sectorId);
        if (!found) {
            EXCEPTION("Unable to populate sector: '{}' not found", sectorId);
        }

        const auto& sector = found.value();

        // std::mt19937_64 rng{sector.seed};

        /*auto skybox = std::make_shared<Entity>();
        skybox->addComponent<ComponentSkybox>(sector.seed);
        skybox->scale(Vector3{1000.0f});
        scene.addEntity(skybox);*/

        /*auto sun = std::make_shared<Entity>();
        sun->addComponent<ComponentDirectionalLight>(Color4{1.0f, 0.9f, 0.8f, 1.0f} * 3.0f);
        sun->translate(Vector3{-2.0f, 2.0f, 2.0f});
        scene.addEntity(sun);*/

        /*auto particles = assetManager.find<AssetParticles>("particles_engine_exhaust_01");
        auto dummy = std::make_shared<Entity>();
        dummy->addComponent<ComponentParticleEmitter>(particles);
        dummy->translate({0.0f, 3.0f, 0.0f});
        scene.addEntity(dummy);

        auto asteroidModel = assetManager.find<AssetModel>("model_asteroid_01_a");

        for (auto y = 0; y < 10; y++) {
            for (auto x = 0; x < 10; x++) {
                auto asteroid = std::make_shared<Entity>();
                asteroid->addComponent<ComponentModel>(asteroidModel);
                asteroid->translate(Vector3{x * 3.0f, 0.0f, y * 3.0f});
                scene.addEntity(asteroid);
            }
        }*/

        /*auto blockEngineHousing = assetManager.find<AssetBlock>("block_engine_housing_t1");
        auto blockEngineNozzle = assetManager.find<AssetBlock>("block_engine_nozzle_t1");
        auto blockFrame = assetManager.find<AssetBlock>("block_frame_t1");
        auto blockBattery = assetManager.find<AssetBlock>("block_battery_t1");
        auto blockSmallReactor = assetManager.find<AssetBlock>("block_reactor_small_t1");
        auto blockTank = assetManager.find<AssetBlock>("block_tank_t1");

        auto dummy = std::make_shared<Entity>();
        auto grid = dummy->addComponent<ComponentGrid>();
        grid->insert(blockEngineHousing, Vector3i{-1, 0, 0}, 0);
        grid->insert(blockEngineHousing, Vector3i{1, 0, 0}, 0);
        grid->insert(blockEngineNozzle, Vector3i{-1, 0, 1}, 0);
        grid->insert(blockEngineNozzle, Vector3i{1, 0, 1}, 0);
        grid->insert(blockFrame, Vector3i{0, 0, 0}, 0);
        grid->insert(blockBattery, Vector3i{-1, 0, -1}, 0);
        grid->insert(blockBattery, Vector3i{1, 0, -1}, 0);
        grid->insert(blockSmallReactor, Vector3i{0, 0, -1}, 0);
        grid->insert(blockTank, Vector3i{-1, 0, -2}, 0);
        grid->insert(blockTank, Vector3i{1, 0, -2}, 0);
        grid->insert(blockFrame, Vector3i{0, 0, -2}, 0);

        auto turretAsset = assetManager.find<AssetTurret>("turret_01");

        auto turret = std::make_shared<Entity>();
        turret->setParent(dummy);
        turret->translate(Vector3{1, 1, -2});
        auto turretCmp = turret->addComponent<ComponentTurret>(turretAsset);
        turretCmp->setTarget(Vector3{-4, 6, -15});

        turret = std::make_shared<Entity>();
        turret->setParent(dummy);
        turret->translate(Vector3{-1, 1, -2});
        turretCmp = turret->addComponent<ComponentTurret>(turretAsset);
        turretCmp->setTarget(Vector3{-4, 6, -15});

        dummy->rotate(Vector3{0.0f, 1.0f, 0.0f}, 45.0f);
        dummy->rotate(Vector3{1.0f, 0.0f, 0.0f}, -25.0f);
        dummy->translate(Vector3{3.0f, 0.0f, 0.0f});

        auto control = dummy->addComponent<ComponentShipControl>();
        control->init();

        scene.addEntity(dummy);*/

        /*dummy = std::make_shared<Entity>();
        dummy->addComponent<ComponentModel>(blockFrame->getModel());
        dummy->translate(turretCmp->getTarget());
        scene.addEntity(dummy);*/
    } catch (...) {
        EXCEPTION_NESTED("Failed to populate sector");
    }
}

void Sector::spawnPlayerShip(SessionPtr session) {
    /*auto blockEngineHousing = assetManager.find<AssetBlock>("block_engine_housing_t1");
    auto blockEngineNozzle = assetManager.find<AssetBlock>("block_engine_nozzle_t1");
    auto blockFrame = assetManager.find<AssetBlock>("block_frame_t1");
    auto blockBattery = assetManager.find<AssetBlock>("block_battery_t1");
    auto blockSmallReactor = assetManager.find<AssetBlock>("block_reactor_small_t1");
    auto blockTank = assetManager.find<AssetBlock>("block_tank_t1");

    auto dummy = std::make_shared<Entity>();
    auto grid = dummy->addComponent<ComponentGrid>();
    grid->insert(blockEngineHousing, Vector3i{-1, 0, 0}, 0);
    grid->insert(blockEngineHousing, Vector3i{1, 0, 0}, 0);
    grid->insert(blockEngineNozzle, Vector3i{-1, 0, 1}, 0);
    grid->insert(blockEngineNozzle, Vector3i{1, 0, 1}, 0);
    grid->insert(blockFrame, Vector3i{0, 0, 0}, 0);
    grid->insert(blockBattery, Vector3i{-1, 0, -1}, 0);
    grid->insert(blockBattery, Vector3i{1, 0, -1}, 0);
    grid->insert(blockSmallReactor, Vector3i{0, 0, -1}, 0);
    grid->insert(blockTank, Vector3i{-1, 0, -2}, 0);
    grid->insert(blockTank, Vector3i{1, 0, -2}, 0);
    grid->insert(blockFrame, Vector3i{0, 0, -2}, 0);

    auto turretAsset = assetManager.find<AssetTurret>("turret_01");

    auto turret = std::make_shared<Entity>();
    turret->setParent(dummy);
    turret->translate(Vector3{1, 1, -2});
    auto turretCmp = turret->addComponent<ComponentTurret>(turretAsset);
    turretCmp->setTarget(Vector3{-4, 6, -15});

    turret = std::make_shared<Entity>();
    turret->setParent(dummy);
    turret->translate(Vector3{-1, 1, -2});
    turretCmp = turret->addComponent<ComponentTurret>(turretAsset);
    turretCmp->setTarget(Vector3{-4, 6, -15});

    dummy->rotate(Vector3{0.0f, 1.0f, 0.0f}, 45.0f);
    dummy->rotate(Vector3{1.0f, 0.0f, 0.0f}, -25.0f);
    dummy->translate(Vector3{3.0f, 0.0f, 0.0f});

    auto control = dummy->addComponent<ComponentShipControl>();
    control->init();

    dummy->addComponent<ComponentPlayer>(session->getPlayerId());

    scene.addEntity(dummy);*/
}

void Sector::update(const float delta) {
    /*for (auto& entity : scene.getEntities()) {
        if (entity->isDirty()) {
            entity->clearDirty();
            entityDeltas.push_back(entity->getDelta());
        }
    }

    for (auto& player : players) {
        MessageSceneEntities::Response msg{};
        for (auto& entity : player.entitiesToSync) {
            msg.entities.push_back(entity);
            if (msg.entities.size() >= 32) {
                break;
            }
        }

        player.entitiesToSync.erase(player.entitiesToSync.begin(), player.entitiesToSync.begin() + msg.entities.size());

        if (!msg.entities.empty()) {
            player.session->send(msg);
        }
    }*/

    /*sync.post([=]() {
        try {
            scene.update(delta);
        } catch (std::exception& e) {
            BACKTRACE(CMP, e, "Sector tick error");
        }
    });*/

    /*while (!entityDeltas.empty()) {
        MessageSceneDeltas::Response msg{};
        for (const auto& d : entityDeltas) {
            msg.deltas.push_back(d);
            if (msg.deltas.size() >= 32) {
                break;
            }
        }

        entityDeltas.erase(entityDeltas.begin(), entityDeltas.begin() + msg.deltas.size());

        if (!msg.deltas.empty()) {
            for (auto& player : players) {
                player.session->send(msg);
            }
        }
    }

    entityDeltas.clear();*/

    sync.reset();
    sync.run();
}

void Sector::addPlayer(SessionPtr session) {
    sync.post([this, session = std::move(session)]() {
        logger.info("Adding player: '{}' to sector: '{}'", session->getPlayerId(), sectorId);

        auto systemData = world.systems.get(galaxyId, systemId);

        players.emplace_back();
        players.back().session = session;

        // Send a message to the player that their location has changed
        MessagePlayerLocationChanged msg{};
        msg.location.galaxyId = getGalaxyId();
        msg.location.systemId = getSystemId();
        msg.location.sectorId = getSectorId();
        msg.systemSeed = systemData.seed;
        session->send(msg);

        // Send an event that a new player has entered the sector
        EventPlayer event{};
        event.playerId = session->getPlayerId();
        eventBus.enqueue("sector_player_added", event);
    });

    /*sync.post([this, session = std::move(session)]() {
        logger.info("Adding player: '{}' to sector: '{}'", session->getPlayerId(), sectorId);

        players.emplace_back();
        players.back().session = session;

        spawnPlayerShip(session);

        const auto op = [](const EntityPtr& entity) {
            // EntityView view;
            // view.ptr = entity;
            return entity;
        };

        std::transform(scene.getEntities().begin(), scene.getEntities().end(),
                       std::back_inserter(players.back().entitiesToSync), op);

        auto location = world.getPlayerLocation(session->getPlayerId());
        MessagePlayerLocationChanged::Response msg{};
        msg.location = location;
        session->send(msg);
    });*/
}

/*void Sector::handle(const SessionPtr& session, MessageShipMovement::Request req, MessageShipMovement::Response& res) {
    sync.post([this, session, req = std::move(req)]() {
        auto& systemPlayers = scene.getComponentSystem<ComponentPlayer>();
        for (const auto& component : systemPlayers) {
            if (component->getPlayerId() == session->getPlayerId()) {
                auto entity = dynamic_cast<Entity*>(&component->getObject());
                if (entity) {
                    auto shipControl = entity->findComponent<ComponentShipControl>();
                    if (shipControl) {
                        shipControl->setMovement(req.left, req.right, req.up, req.down);
                    }
                }
                break;
            }
        }
    });
}*/
