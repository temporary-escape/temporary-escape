#pragma once

#include "../Scene/Scene.hpp"
#include "Services.hpp"

namespace Engine {
class ENGINE_API SectorLoader {
public:
    explicit SectorLoader(const Config& config, AssetManager& assetManager, Database& db, Services& services);

    void populate(const std::string& galaxyId, const std::string& systemId, const std::string& sectorId, Scene& scene);

private:
    const Config& config;
    AssetManager& assetManager;
    Database& db;
    Services& services;
};
} // namespace Engine
