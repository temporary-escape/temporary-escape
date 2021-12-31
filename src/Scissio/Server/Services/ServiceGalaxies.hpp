#pragma once

#include "Service.hpp"

namespace Scissio {
class ServiceGalaxies : public Service {
public:
    explicit ServiceGalaxies(const Config& config, AssetManager& assetManager, Database& db);
    ~ServiceGalaxies() override = default;

    void generate(uint64_t seed);
    std::vector<GalaxyData> getForPlayer(const std::string& playerId);

    void tick() override;

private:
    const Config& config;
    AssetManager& assetManager;
    Database& db;
};
} // namespace Scissio
