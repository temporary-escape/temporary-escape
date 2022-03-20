#pragma once

#include "../Assets/AssetPlanet.hpp"
#include "../Utils/Database.hpp"
#include "../Utils/Msgpack.hpp"

namespace Engine {
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
    std::string id;
    std::string galaxyId;
    std::string systemId;
    std::string sectorId;

    SCHEMA_DEFINE(id, galaxyId, systemId, sectorId);
    SCHEMA_INDEXES_NONE();
    SCHEMA_NAME("PlayerLocationData");
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
    std::string galaxyId;
    std::string name;
    Vector2 pos;
    uint64_t seed = 0;

    SCHEMA_DEFINE(id, galaxyId, name, pos, seed);
    SCHEMA_INDEXES(name);
    SCHEMA_NAME("RegionData");
};

SCHEMA(SystemData) {
    std::string id;
    std::string galaxyId;
    std::string regionId;
    std::string name;
    Vector2 pos;
    uint64_t seed = 0;
    std::vector<std::string> connections;

    SCHEMA_DEFINE(id, galaxyId, regionId, name, pos, seed, connections);
    SCHEMA_INDEXES(name);
    SCHEMA_NAME("SystemData");
};

SCHEMA(SectorData) {
    std::string id;
    std::string galaxyId;
    std::string systemId;
    std::string name;
    Vector2 pos;
    uint64_t seed = 0;
    bool generated = false;

    SCHEMA_DEFINE(id, galaxyId, systemId, name, pos, seed, generated);
    SCHEMA_INDEXES(name);
    SCHEMA_NAME("SectorData");
};

SCHEMA(SectorPlanetData) {
    std::string id;
    std::string galaxyId;
    std::string systemId;
    std::string name;
    bool isMoon{false};
    std::optional<std::string> planet{std::nullopt};
    Vector2 pos;
    AssetPlanetPtr asset;

    SCHEMA_DEFINE(id, name, isMoon, planet, pos, asset);
    SCHEMA_INDEXES_NONE();
    SCHEMA_NAME("SectorPlanetData");
};
} // namespace Engine
