#pragma once

#include "Service.hpp"

namespace Engine {
class ENGINE_API ServiceSectors : public Service {
public:
    explicit ServiceSectors(const Config& config, AssetManager& assetManager, Database& db);
    ~ServiceSectors() override = default;

    void generate();
    std::optional<SectorData> find(const std::string& galaxyId, const std::string& systemId,
                                   const std::string& sectorId);
    void generate(const std::string& galaxyId);
    void generate(const std::string& galaxyId, const std::string& systemId);
    void createSectorPlanet(const SectorPlanetData& planet);
    void createSector(const SectorData& sector);
    void tick() override;

private:
    const Config& config;
    AssetManager& assetManager;
    Database& db;
};
} // namespace Engine
