#pragma once

#include "Service.hpp"

namespace Engine {
class ENGINE_API ServiceGalaxies : public Service {
public:
    explicit ServiceGalaxies(const Config& config, AssetManager& assetManager, Database& db);
    ~ServiceGalaxies() override = default;

    void generate(uint64_t seed);
    GalaxyData getForPlayer(const std::string& playerId, const std::string& id);
    void createGalaxy(const GalaxyData& galaxy);

    void tick() override;

private:
    const Config& config;
    AssetManager& assetManager;
    Database& db;
};
} // namespace Engine
