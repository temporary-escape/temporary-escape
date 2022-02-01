#pragma once

#include "../Assets/AssetPlanet.hpp"
#include "../Utils/Msgpack.hpp"
#include "Database.hpp"

namespace Engine {
struct PlayerData {
    std::string id;
    uint64_t secret;
    std::string name;
    bool admin;

    MSGPACK_DEFINE_MAP(id, secret, name, admin);
};

SCHEMA_DEFINE_INDEXED(PlayerData, secret);

struct PlayerLocationData {
    std::string id;
    std::string galaxyId;
    std::string systemId;
    std::string sectorId;

    MSGPACK_DEFINE_MAP(id, galaxyId, systemId, sectorId);
};

SCHEMA_DEFINE(PlayerLocationData);

struct GalaxyData {
    std::string id;
    std::string name;
    Vector2 pos;
    uint64_t seed = 0;

    MSGPACK_DEFINE_MAP(id, name, pos, seed);
};

SCHEMA_DEFINE_INDEXED(GalaxyData, name);

struct RegionData {
    std::string id;
    std::string galaxyId;
    std::string name;
    Vector2 pos;
    uint64_t seed = 0;

    MSGPACK_DEFINE_MAP(id, galaxyId, name, pos, seed);
};

SCHEMA_DEFINE_INDEXED(RegionData, name);

struct SystemData {
    std::string id;
    std::string galaxyId;
    std::string regionId;
    std::string name;
    Vector2 pos;
    uint64_t seed = 0;
    std::vector<std::string> connections;

    MSGPACK_DEFINE_MAP(id, galaxyId, regionId, name, pos, seed, connections);
};

SCHEMA_DEFINE_INDEXED(SystemData, name);

struct SectorData {
    std::string id;
    std::string galaxyId;
    std::string systemId;
    std::string name;
    Vector2 pos;
    uint64_t seed = 0;
    bool generated = false;

    MSGPACK_DEFINE_MAP(id, galaxyId, systemId, name, pos, seed, generated);
};

SCHEMA_DEFINE_INDEXED(SectorData, name);

struct SectorPlanetData {
    std::string id;
    std::string name;
    bool isMoon{false};
    std::optional<std::string> planet{std::nullopt};
    Vector2 pos;
    AssetPlanetPtr asset;

    MSGPACK_DEFINE_MAP(id, name, isMoon, planet, pos, asset);
};

SCHEMA_DEFINE(SectorPlanetData);
} // namespace Engine