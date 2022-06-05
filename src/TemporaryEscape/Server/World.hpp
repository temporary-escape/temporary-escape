#pragma once

#include "../Assets/AssetManager.hpp"
#include "../Config.hpp"
#include "../Utils/Database.hpp"
#include "Schemas.hpp"

namespace Engine {
class ENGINE_API World {
public:
    explicit World(const Config& config, AssetManager& assetManager, TransactionalDatabase& db);
    ~World() = default;

    // Players
    std::optional<std::string> playerSecretToId(uint64_t);
    PlayerData loginPlayer(uint64_t secret, const std::string& name);
    PlayerLocationData findPlayerStartingLocation(const std::string& playerId);
    PlayerLocationData getPlayerLocation(const std::string& playerId);
    std::vector<AssetBlockPtr> getPlayerUnlockedBlocks(const std::string& playerId);

    // Galaxies
    GalaxyData getGalaxyForPlayer(const std::string& playerId, const std::string& galaxyId);
    void create(const GalaxyData& galaxy);

    // Regions
    void create(const RegionData& region);
    std::vector<RegionData> getRegionsForPlayer(const std::string& playerId, const std::string& galaxyId,
                                                const std::string& start, std::string& next);

    // Factions
    void create(const FactionData& faction);
    std::vector<FactionData> getFactionsForPlayer(const std::string& playerId, const std::string& galaxyId,
                                                  const std::string& start, std::string& next);

    // Systems
    std::vector<SystemData> getSystemsForPlayer(const std::string& playerId, const std::string& galaxyId,
                                                const std::string& start, std::string& next);
    void create(const SystemData& system);
    std::vector<SectorPlanetData> getSystemPlanets(const std::string& galaxyId, const std::string& systemId);
    void tick();

    // Sectors
    std::optional<SectorData> findSector(const std::string& galaxyId, const std::string& systemId,
                                         const std::string& sectorId);
    void create(const SectorPlanetData& planet);
    void create(const SectorData& sector);

private:
    const Config& config;
    AssetManager& assetManager;
    TransactionalDatabase& db;
};
} // namespace Engine
