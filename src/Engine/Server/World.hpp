#pragma once

#include "../Assets/Registry.hpp"
#include "../Config.hpp"
#include "../Database/Database.hpp"
#include "Schemas.hpp"

namespace Engine {
class ENGINE_API World {
public:
    explicit World(const Config& config, Registry& registry, TransactionalDatabase& db);
    ~World() = default;

    class Players {
    public:
        explicit Players(const Config& config, Registry& registry, TransactionalDatabase& db);

        std::optional<std::string> secretToId(uint64_t);
        PlayerData login(uint64_t secret, const std::string& name);
        PlayerLocationData findStartingLocation(const std::string& playerId);
        PlayerLocationData getLocation(const std::string& playerId);
        std::vector<BlockPtr> getUnlockedBlocks(const std::string& playerId);

    private:
        const Config& config;
        Registry& registry;
        TransactionalDatabase& db;
    } players;

    class Galaxies {
    public:
        explicit Galaxies(const Config& config, Registry& registry, TransactionalDatabase& db);

        GalaxyData getForPlayer(const std::string& playerId, const std::string& galaxyId);
        void create(const GalaxyData& galaxy);

    private:
        const Config& config;
        Registry& registry;
        TransactionalDatabase& db;
    } galaxies;

    class Regions {
    public:
        explicit Regions(const Config& config, Registry& registry, TransactionalDatabase& db);

        void create(const RegionData& region);
        std::vector<RegionData> getForPlayer(const std::string& playerId, const std::string& galaxyId,
                                             const std::string& start, std::string& next);

    private:
        const Config& config;
        Registry& registry;
        TransactionalDatabase& db;
    } regions;

    class Factions {
    public:
        explicit Factions(const Config& config, Registry& registry, TransactionalDatabase& db);

        void create(const FactionData& faction);
        std::vector<FactionData> getForPlayer(const std::string& playerId, const std::string& galaxyId,
                                              const std::string& start, std::string& next);

    private:
        const Config& config;
        Registry& registry;
        TransactionalDatabase& db;
    } factions;

    class Systems {
    public:
        explicit Systems(const Config& config, Registry& registry, TransactionalDatabase& db);

        std::vector<SystemData> getForPlayer(const std::string& playerId, const std::string& galaxyId,
                                             const std::string& start, std::string& next);
        void create(const SystemData& system);
        std::vector<SectorPlanetData> getPlanets(const std::string& galaxyId, const std::string& systemId);

    private:
        const Config& config;
        Registry& registry;
        TransactionalDatabase& db;
    } systems;

    class Sectors {
    public:
        explicit Sectors(const Config& config, Registry& registry, TransactionalDatabase& db);

        std::optional<SectorData> find(const std::string& galaxyId, const std::string& systemId,
                                       const std::string& sectorId);
        void create(const SectorPlanetData& planet);
        void create(const SectorData& sector);

    private:
        const Config& config;
        Registry& registry;
        TransactionalDatabase& db;
    } sectors;
};
} // namespace Engine
