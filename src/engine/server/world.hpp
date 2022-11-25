#pragma once

#include "../services/service_factions.hpp"
#include "../services/service_galaxies.hpp"
#include "../services/service_players.hpp"
#include "../services/service_regions.hpp"
#include "../services/service_sectors.hpp"
#include "../services/service_systems.hpp"

namespace Engine {
SCHEMA(SaveFileData) {
    uint64_t seed = 0;
    bool generated = false;

    SCHEMA_DEFINE(seed, generated);
    SCHEMA_INDEXES_NONE();
    SCHEMA_NAME("SaveFileData");
};

class ENGINE_API World {
public:
    explicit World(const Config& config, Registry& registry, TransactionalDatabase& db, MsgNet::Server& server,
                   Service::SessionValidator& sessionValidator);
    ~World() = default;

    ServicePlayers players;
    ServiceGalaxies galaxies;
    ServiceRegions regions;
    ServiceFactions factions;
    ServiceSystems systems;
    ServiceSectors sectors;
};
} // namespace Engine
