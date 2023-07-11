#include "sector.hpp"
#include "../scene/controllers/controller_network.hpp"
#include "../utils/string_utils.hpp"
#include "server.hpp"
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

    scene = std::make_unique<Scene>(config, true);
    lua = std::make_unique<Lua>(config, eventBus);

    const auto galaxyData = db.get<GalaxyData>(galaxyId);
    const auto systemData = db.get<SystemData>(fmt::format("{}/{}", galaxyId, systemId));
    const auto sectorData = db.get<SectorData>(fmt::format("{}/{}/{}", galaxyId, systemId, sectorId));

    std::mt19937_64 rng{sectorData.seed};

    lua->require(sectorData.luaTemplate, [&](sol::table& table) {
        auto res = table["populate"](table, rng, galaxyData, systemData, sectorData, scene.get());
        if (!res.valid()) {
            sol::error err = res;
            EXCEPTION("Failed to call sector template: '{}' error: {}", sectorData.luaTemplate, err.what());
        }
    });
    /*try {
        auto found = world.sectors.find(galaxyId, systemId, sectorId);
        if (!found) {
            EXCEPTION("Unable to load sector: '{}' not found", sectorId);
        }

        const auto& sector = found.value();

    } catch (...) {
        EXCEPTION_NESTED("Failed to load sector");
    }*/

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

        // Send a message to the player that their location has changed
        MessagePlayerLocationEvent msg{};
        msg.location.galaxyId = getGalaxyId();
        msg.location.systemId = getSystemId();
        msg.location.sectorId = getSectorId();
        session->send(msg);

        // Send an event that a new player has entered the sector
        // EventPlayer event{};
        // event.playerId = session->getPlayerId();
        // TODO:
        // eventBus.enqueue("sector_player_added", event);

        logger.info("Player: '{}' added to sector: '{}'", session->getPlayerId(), sectorId);

        // Send all entities to the player
        worker.post([this, session]() {
            const auto peer = session->getStream();
            if (peer) {
                scene->getController<ControllerNetwork>().sendFullSnapshot(*peer);
            }
        });
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
