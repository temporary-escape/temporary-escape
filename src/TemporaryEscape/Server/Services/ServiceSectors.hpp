#pragma once

#include "Service.hpp"

namespace Engine {
class ServiceSectors : public Service {
public:
    explicit ServiceSectors(const Config& config, AssetManager& assetManager, Database& db);
    ~ServiceSectors() override = default;

    void generate();
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
