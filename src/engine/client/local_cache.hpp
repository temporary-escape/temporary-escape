#pragma once

#include "../server/messages.hpp"
#include "../server/schemas.hpp"
#include "../server/services/service_factions.hpp"
#include "../server/services/service_galaxy.hpp"
#include "../server/services/service_regions.hpp"
#include "../server/services/service_systems.hpp"

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
};
} // namespace Engine
