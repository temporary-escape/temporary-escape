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
    struct {
        EntityId entity{NullEntity};
        Vector3 position;
        EntityId approaching{NullEntity};
        float approachDistance{0.0f};
        float orbitRadius{0.0f};
        float keepAtDistance{0.0f};
        float forwardVelocity{0.0f};
        float forwardVelocityMax{0.0f};
    } player;
};
} // namespace Engine
