#pragma once

#include "Service.hpp"

namespace Engine {
class ENGINE_API ServiceSystems : public Service {
public:
    explicit ServiceSystems(const Config& config, AssetManager& assetManager, Database& db);
    ~ServiceSystems() override = default;

    std::vector<SystemData> getForPlayer(const std::string& playerId, const std::string& galaxyId,
                                         const std::string& start, std::string& next);
    void generate();
    void generate(const std::string& galaxyId);
    void generate(const std::string& galaxyId, const std::string& systemId);
    void createSystem(const SystemData& system);
    std::vector<SectorPlanetData> getSystemPlanets(const std::string& galaxyId, const std::string& systemId);
    void tick() override;

private:
    const Config& config;
    AssetManager& assetManager;
    Database& db;
};
} // namespace Engine
