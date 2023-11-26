#pragma once

#include "../Server/Messages.hpp"
#include "../Server/Schemas.hpp"
#include "../Server/Services/ServiceFactions.hpp"
#include "../Server/Services/ServiceGalaxy.hpp"
#include "../Server/Services/ServiceRegions.hpp"
#include "../Server/Services/ServiceSystems.hpp"

namespace Engine {
struct LocalCache {
    // Player information
    std::string playerId;
    PlayerLocationData location;

    // World map information
    struct {
        std::string name;
        std::string id;

        std::unordered_map<std::string, RegionData> regions;
        std::unordered_map<std::string, SystemData> systems;
        std::unordered_map<std::string, FactionData> factions;
        std::vector<const SystemData*> systemsOrdered;
    } galaxy;

    // Scene information
    Entity playerEntityId;
};
} // namespace Engine
