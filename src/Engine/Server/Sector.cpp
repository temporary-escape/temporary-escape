#include "Sector.hpp"
#include "../Scene/Controllers/ControllerNetwork.hpp"
#include "../Scene/Controllers/ControllerPathfinding.hpp"
#include "../Utils/StringUtils.hpp"
#include "Server.hpp"
#include <sol/sol.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

Sector::Sector(const Config& config, Database& db, AssetsManager& assetsManager, EventBus& eventBus,
               std::string galaxyId, std::string systemId, std::string sectorId) :
    config{config},
    db{db},
    assetsManager{assetsManager},
    eventBus{eventBus},
    galaxyId{std::move(galaxyId)},
    systemId{std::move(systemId)},
    sectorId{std::move(sectorId)},
    loaded{false},
    rng{std::random_device()()} {

    logger.info("Started sector: '{}'", this->sectorId);
}

Sector::~Sector() {
    logger.info("Stopping sector: '{}'", sectorId);

    players.clear();
    scene.reset();
    lua.reset();

    logger.info("Stopped sector: '{}'", sectorId);
}

void Sector::load() {
    logger.info("Sector is loading: '{}'", sectorId);

    if (loaded) {
        EXCEPTION("Sector was already loaded id: '{}'", sectorId);
    }

    lua = std::make_unique<Lua>(config, eventBus);
    scene = std::make_unique<Scene>(config, nullptr, lua.get());
    lua->setScene(*scene);

    const auto galaxyData = db.get<GalaxyData>(galaxyId);
    const auto systemData = db.get<SystemData>(fmt::format("{}/{}", galaxyId, systemId));
    const auto sectorData = db.get<SectorData>(fmt::format("{}/{}/{}", galaxyId, systemId, sectorId));

    lua->root()["get_sector_data"] = [this]() {
        return db.get<SectorData>(fmt::format("{}/{}/{}", galaxyId, systemId, sectorId));
    };

    lua->require("base.sector");
    // generator.populate(sectorData, *scene);
    /*lua->require(sectorData.luaTemplate, [&](sol::table& table) {
        auto res = table["populate"](table, rng, galaxyData, systemData, sectorData, scene.get());
        if (!res.valid()) {
            sol::error err = res;
            EXCEPTION("Failed to call sector template: '{}' error: {}", sectorData.luaTemplate, err.what());
        }
    });*/

    // Build pathfinding
    scene->getDynamicsWorld().updateAabbs();
    // scene->getController<ControllerPathfinding>().buildTree();
    scene->createEntityFrom(sectorData.entity);

    loaded = true;
    logger.info("Sector is loaded: '{}'", sectorId);
}

void Sector::update() {
    try {
        worker.poll();

        const auto tickF = static_cast<float>(config.tickLengthUs.count()) / 1000000.0f;
        scene->update(tickF);

        // if (tickCount % 2 == 0 && tickCount != 0) {
        // const auto t0 = std::chrono::steady_clock::now();

        auto& networkController = scene->getController<ControllerNetwork>();
        for (const auto& player : players) {
            if (const auto stream = player->getStream(); stream) {
                networkController.sendUpdate(*stream);
            }
        }
        networkController.resetUpdates();
        //}

        // const auto t1 = std::chrono::steady_clock::now();
        // const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
        // logger.info("Network update took: {} ms", ms);

        ++tickCount;
    } catch (...) {
        EXCEPTION_NESTED("Failed to update sector: {}", sectorId);
    }
}

void Sector::addPlayer(const SessionPtr& session) {
    worker.postSafe([this, session]() {
        const auto it = std::find_if(players.begin(), players.end(), [&](const SessionPtr& p) { return p == session; });
        if (it != players.end()) {
            EXCEPTION("Player: {} is already in sector: {}", session->getPlayerId(), sectorId);
        }
        players.push_back(session);

        auto systemData = db.get<SystemData>(fmt::format("{}/{}", galaxyId, systemId));
        auto sectorData = db.get<SectorData>(fmt::format("{}/{}/{}", galaxyId, systemId, sectorId));

        // Spawn player entity
        auto playerEntityId = spawnPlayerEntity(session);
        // playerEntity.addComponent<ComponentShipControl>();

        // Remember that this player controls this entity
        playerControl.emplace(session, playerEntityId);

        // Send a message to the player that their location has changed
        MessagePlayerLocationEvent msg{};
        msg.location.galaxyId = getGalaxyId();
        msg.location.systemId = getSystemId();
        msg.location.sectorId = getSectorId();
        msg.sector = sectorData;
        msg.system = systemData;
        session->send(msg);

        logger.info("Player: '{}' added to sector: '{}'", session->getPlayerId(), sectorId);

        // Publish an event
        EventData event{};
        event["player_id"] = session->getPlayerId();
        event["sector_id"] = sectorData.id;
        eventBus.enqueue("sector_player_added", event);

        // Send all entities to the player
        worker.postSafe([this, session, playerEntityId]() {
            const auto peer = session->getStream();
            if (peer) {
                scene->getController<ControllerNetwork>().sendFullSnapshot(*peer);

                // Let the player know which entity they control
                MessagePlayerControlEvent msg{};
                msg.entityId = playerEntityId;
                session->send(msg);
            }
        });
    });
}

void Sector::removePlayer(const SessionPtr& session) {
    worker.postSafe([this, session]() {
        const auto it = std::find_if(players.begin(), players.end(), [&](const SessionPtr& p) { return p == session; });
        if (it != players.end()) {
            players.erase(it);
        }
    });
}

EntityId Sector::spawnPlayerEntity(const SessionPtr& session) {
    auto table = scene->getLua().getState().create_table();
    table["player_id"] = session->getPlayerId();

    auto entity = scene->createEntityFrom("player", table);
    auto* transform = scene->tryGetComponent<ComponentTransform>(entity);
    if (transform) {
        transform->move(Vector3{10000.0f, 0.0f, 0.0f});
    }

    return entity;
}

void Sector::handleShipAction(const SessionPtr& session, std::function<void(Entity&, ComponentShipControl&)> callback) {
    worker.postSafe([this, session, c = std::move(callback)]() {
        // Find the entity that the player controls
        const auto found = playerControl.find(session);
        if (found == playerControl.end()) {
            return;
        }

        // Get the ship control of the player's entity
        auto entity = scene->fromHandle(found->second);
        auto* shipControl = entity.tryGetComponent<ComponentShipControl>();
        if (!shipControl) {
            return;
        }

        c(entity, *shipControl);
    });
}

void Sector::handle(const SessionPtr& session, MessageActionApproach req) {
    handleShipAction(session, [req](Entity& entity, ComponentShipControl& shipControl) {
        logger.debug("Entity: {} approaching: {}", entity.getHandle(), req.entityId);
        shipControl.actionApproach(req.entityId);
    });
}

void Sector::handle(const SessionPtr& session, MessageActionOrbit req) {
    handleShipAction(session, [req](Entity& entity, ComponentShipControl& shipControl) {
        logger.debug("Entity: {} orbiting: {}", entity.getHandle(), req.entityId);
        shipControl.actionOrbit(req.entityId, req.radius);
    });
}

void Sector::handle(const SessionPtr& session, MessageActionKeepDistance req) {
    handleShipAction(session, [req](Entity& entity, ComponentShipControl& shipControl) {
        logger.debug("Entity: {} keeping distance: {}", entity.getHandle(), req.entityId);
        shipControl.actionKeepDistance(req.entityId, req.distance);
    });
}

void Sector::handle(const SessionPtr& session, MessageActionStopMovement req) {
    handleShipAction(session, [req](Entity& entity, ComponentShipControl& shipControl) {
        logger.debug("Entity: {} stopping", entity.getHandle());
        shipControl.actionStopMovement();
    });
}

void Sector::handle(const SessionPtr& session, MessageActionGoDirection req) {
    handleShipAction(session, [req](Entity& entity, ComponentShipControl& shipControl) {
        logger.debug("Entity: {} going direction: {}", entity.getHandle(), req.direction);
        shipControl.actionGoDirection(req.direction);
    });
}

void Sector::handle(const SessionPtr& session, MessageActionWarpTo req) {
    handleShipAction(session, [this, req](Entity& entity, ComponentShipControl& shipControl) {
        if (req.sectorId != sectorId) {
            const auto thisSector = db.find<SectorData>(fmt::format("{}/{}/{}", galaxyId, systemId, sectorId));
            const auto targetSector = db.find<SectorData>(fmt::format("{}/{}/{}", galaxyId, systemId, req.sectorId));

            if (thisSector && targetSector) {
                const auto dir = targetSector->pos - thisSector->pos;

                logger.debug("Entity: {} warping to: {}", entity.getHandle(), req.sectorId);
                shipControl.actionWarpTo(Vector3{dir.x, 0.0f, dir.y});
            }
        }
    });
}

void Sector::handle(const SessionPtr& session, MessageControlTargetEvent req) {
    worker.postSafe([this, session, req]() {
        // Find the entity that the player controls
        const auto found = playerControl.find(session);
        if (found != playerControl.end()) {
            const auto entity = scene->fromHandle(found->second);
            auto* shipControl = entity.tryGetComponent<ComponentShipControl>();

            const auto target = scene->fromHandle(static_cast<entt::entity>(req.entityId));
            if (!target) {
                logger.warn("Can not target entity: {} invalid entity", req.entityId);
                return;
            }

            if (const auto* targetTransform = target.tryGetComponent<ComponentTransform>(); targetTransform) {
                for (auto* turret : shipControl->getTurrets()) {
                    turret->setTarget(targetTransform);
                }
            } else {
                logger.warn("Can not target entity: {} no transform found", req.entityId);
            }
        }
    });
}
