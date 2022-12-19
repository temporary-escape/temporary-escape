#pragma once

#include "../services/service_factions.hpp"
#include "../services/service_galaxies.hpp"
#include "../services/service_players.hpp"
#include "../services/service_regions.hpp"
#include "../services/service_sectors.hpp"
#include "../services/service_systems.hpp"

namespace Engine {
SCHEMA(IndexData) {
    uint64_t seed{0};
    std::string galaxyId;

    SCHEMA_DEFINE(seed, galaxyId);
    SCHEMA_INDEXES_NONE();
    SCHEMA_NAME("IndexData");
};

class ENGINE_API World {
public:
    explicit World(const Config& config, Registry& registry, TransactionalDatabase& db, Network::Server& server,
                   Service::SessionValidator& sessionValidator);
    ~World() = default;

    IndexData getIndex();
    IndexData updateIndex(const std::function<void(IndexData&)>& callback);

    ServicePlayers players;
    ServiceGalaxies galaxies;
    ServiceRegions regions;
    ServiceFactions factions;
    ServiceSystems systems;
    ServiceSectors sectors;

private:
    TransactionalDatabase& db;
};
} // namespace Engine
