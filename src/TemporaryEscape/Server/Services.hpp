#pragma once

#include "Services/ServiceGalaxies.hpp"
#include "Services/ServicePlayers.hpp"
#include "Services/ServiceRegions.hpp"
#include "Services/ServiceSectors.hpp"
#include "Services/ServiceSystems.hpp"

namespace Engine {
struct ENGINE_API Services {
    explicit Services(const Config& config, AssetManager& assetManager, Database& db)
        : galaxies(config, assetManager, db), regions(config, assetManager, db), systems(config, assetManager, db),
          sectors(config, assetManager, db), players(config, assetManager, db) {
    }

    ServiceGalaxies galaxies;
    ServiceRegions regions;
    ServiceSystems systems;
    ServiceSectors sectors;
    ServicePlayers players;
};
} // namespace Engine
