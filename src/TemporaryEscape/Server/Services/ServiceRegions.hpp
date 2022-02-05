#pragma once

#include "Service.hpp"

namespace Engine {
class ServiceRegions : public Service {
public:
    explicit ServiceRegions(const Config& config, AssetManager& assetManager, Database& db);
    ~ServiceRegions() override = default;

    void generate();
    void generate(const std::string& galaxyId);
    void createRegion(const RegionData& region);
    void tick() override;
    std::vector<RegionData> getForPlayer(const std::string& playerId, const std::string& galaxyId,
                                         const std::string& start, std::string& next);

private:
    const Config& config;
    AssetManager& assetManager;
    Database& db;
};
} // namespace Engine
