#include "sector.hpp"
#include "../scene/controllers/controller_network.hpp"
#include "../utils/string_utils.hpp"
#include "server.hpp"
#include <sol/sol.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

Sector::Sector(const Config& config, Database& db, AssetsManager& assetsManager, EventBus& eventBus,
               Generator& generator, std::string galaxyId, std::string systemId, std::string sectorId) :
    config{config},
    db{db},
    assetsManager{assetsManager},
    eventBus{eventBus},
    generator{generator},
    galaxyId{std::move(galaxyId)},
    systemId{std::move(systemId)},
    sectorId{std::move(sectorId)},
    loaded{false} {

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

    lua->root()["get_scene"] = [this]() { return scene.get(); };

    const auto galaxyData = db.get<GalaxyData>(galaxyId);
    const auto systemData = db.get<SystemData>(fmt::format("{}/{}", galaxyId, systemId));
    const auto sectorData = db.get<SectorData>(fmt::format("{}/{}/{}", galaxyId, systemId, sectorId));

    lua->root()["get_sector_data"] = [this]() {
        return db.get<SectorData>(fmt::format("{}/{}/{}", galaxyId, systemId, sectorId));
    };

    lua->require("base.sector");
    generator.populate(sectorData, *scene);
    /*lua->require(sectorData.luaTemplate, [&](sol::table& table) {
        auto res = table["populate"](table, rng, galaxyData, systemData, sectorData, scene.get());
        if (!res.valid()) {
            sol::error err = res;
            EXCEPTION("Failed to call sector template: '{}' error: {}", sectorData.luaTemplate, err.what());
        }
    });*/

    loaded = true;
    logger.info("Sector is loaded: '{}'", sectorId);
}

void Sector::update() {
    try {
        worker.poll();

        const auto tickF = static_cast<float>(config.tickLengthUs.count()) / 1000000.0f;
        scene->update(tickF);

        if (tickCount % 10 == 0 && tickCount != 0) {
            auto& networkController = scene->getController<ControllerNetwork>();
            for (const auto& player : players) {
                if (const auto stream = player->getStream(); stream) {
                    networkController.sendUpdate(*stream);
                }
            }
            networkController.resetUpdates();
        }

        ++tickCount;
    } catch (...) {
        EXCEPTION_NESTED("Failed to update sector: {}", sectorId);
    }
}

void Sector::addPlayer(const SessionPtr& session) {
    worker.post([this, session]() {
        const auto it = std::find_if(players.begin(), players.end(), [&](const SessionPtr& p) { return p == session; });
        if (it != players.end()) {
            EXCEPTION("Player: {} is already in sector: {}", session->getPlayerId(), sectorId);
        }
        players.push_back(session);

        auto systemData = db.get<SystemData>(fmt::format("{}/{}", galaxyId, systemId));
        auto sectorData = db.get<SectorData>(fmt::format("{}/{}/{}", galaxyId, systemId, sectorId));

        // Send a message to the player that their location has changed
        MessagePlayerLocationEvent msg{};
        msg.location.galaxyId = getGalaxyId();
        msg.location.systemId = getSystemId();
        msg.location.sectorId = getSectorId();
        msg.sector = sectorData;
        session->send(msg);

        logger.info("Player: '{}' added to sector: '{}'", session->getPlayerId(), sectorId);

        // Publish an event
        EventData event{};
        event["player_id"] = session->getPlayerId();
        event["sector_id"] = sectorData.id;
        eventBus.enqueue("sector_player_added", event);

        // Send all entities to the player
        worker.post([this, session]() {
            const auto peer = session->getStream();
            if (peer) {
                scene->getController<ControllerNetwork>().sendFullSnapshot(*peer);
            }
        });
    });
}

void Sector::removePlayer(const SessionPtr& session) {
    worker.post([this, session]() {
        const auto it = std::find_if(players.begin(), players.end(), [&](const SessionPtr& p) { return p == session; });
        if (it != players.end()) {
            players.erase(it);
        }
    });
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
