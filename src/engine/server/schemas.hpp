#pragma once

#include "../assets/assets_manager.hpp"
#include "../config.hpp"
#include "../database/database.hpp"
#include "../math/vector.hpp"

namespace Engine {
using MetaDataValue = std::variant<bool, std::string, int64_t, double>;

SCHEMA(MetaData) {
    MetaDataValue value;

    SCHEMA_DEFINE(value);
    SCHEMA_INDEXES_NONE();
    SCHEMA_NAME("MetaData");
};

SCHEMA(PlayerData) {
    std::string id;
    uint64_t secret;
    std::string name;
    bool admin;

    SCHEMA_DEFINE(id, secret, name, admin);
    SCHEMA_INDEXES(secret);
    SCHEMA_NAME("PlayerData");
};

SCHEMA(PlayerLocationData) {
    std::string galaxyId;
    std::string systemId;
    std::string sectorId;

    SCHEMA_DEFINE(galaxyId, systemId, sectorId);
    SCHEMA_INDEXES_NONE();
    SCHEMA_NAME("PlayerLocationData");
};

SCHEMA(FactionData) {
    std::string id;
    std::string name;
    float color{0.0f};
    std::string homeGalaxyId;
    std::string homeSystemId;

    SCHEMA_DEFINE(id, name, color, homeGalaxyId, homeSystemId);
    SCHEMA_INDEXES(name);
    SCHEMA_NAME("FactionData");
};

SCHEMA(GalaxyData) {
    std::string id;
    std::string name;
    Vector2 pos;
    uint64_t seed = 0;

    SCHEMA_DEFINE(id, name, pos, seed);
    SCHEMA_INDEXES(name);
    SCHEMA_NAME("GalaxyData");
};

SCHEMA(RegionData) {
    std::string id;
    std::string name;
    Vector2 pos;

    SCHEMA_DEFINE(id, name, pos);
    SCHEMA_INDEXES(name);
    SCHEMA_NAME("RegionData");
};

SCHEMA(SectorData) {
    std::string id;
    std::string galaxyId;
    std::string systemId;
    std::string name;
    Vector2 pos{0.0f, 0.0f};
    uint64_t seed{0};

    SCHEMA_DEFINE(id, galaxyId, systemId, name, pos, seed);
    SCHEMA_INDEXES(name);
    SCHEMA_NAME("SectorData");
};

SCHEMA(SystemData) {
    std::string id;
    std::string galaxyId;
    std::string regionId;
    std::string name;
    std::optional<std::string> factionId;
    Vector2 pos;
    uint64_t seed{0};
    std::vector<std::string> connections;

    SCHEMA_DEFINE(id, galaxyId, regionId, name, factionId, pos, seed, connections);
    SCHEMA_INDEXES(name);
    SCHEMA_NAME("SystemData");
};

SCHEMA(PlanetData) {
    std::string id;
    std::string galaxyId;
    std::string systemId;
    std::string name;
    std::optional<std::string> parent;
    PlanetTypePtr type;
    Vector2 pos;
    uint64_t seed{0};
    float radius{0.5f};

    SCHEMA_DEFINE(id, galaxyId, systemId, name, parent, type, pos, seed, radius);
    SCHEMA_INDEXES_NONE();
    SCHEMA_NAME("PlanetData");
};

ENGINE_API void bindSchemas(Lua& lua);
} // namespace Engine
