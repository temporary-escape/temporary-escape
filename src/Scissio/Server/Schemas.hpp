#pragma once

#include "../Utils/Msgpack.hpp"
#include "Database.hpp"

namespace Scissio {
struct PlayerData {
    std::string id;
    uint64_t secret;
    std::string name;
    bool admin;

    MSGPACK_DEFINE_MAP(id, secret, name, admin);
};

SCHEMA_DEFINE_INDEXED(PlayerData, secret);

struct WorldData {
    uint64_t seed = 0;

    MSGPACK_DEFINE(seed);
};

SCHEMA_DEFINE(WorldData);

struct GalaxyData {
    std::string id;
    std::string name;
    Vector2 pos;
    uint64_t seed = 0;

    MSGPACK_DEFINE(id, name, pos, seed);
};

SCHEMA_DEFINE_INDEXED(GalaxyData, name);

struct RegionData {
    std::string id;
    std::string galaxyId;
    std::string name;
    Vector2 pos;
    uint64_t seed = 0;

    MSGPACK_DEFINE(id, galaxyId, name, pos, seed);
};

SCHEMA_DEFINE_INDEXED(RegionData, name);

struct SystemData {
    std::string id;
    std::string galaxyId;
    std::string regionId;
    std::string name;
    Vector2 pos;
    uint64_t seed = 0;

    MSGPACK_DEFINE(id, galaxyId, regionId, name, pos, seed);
};

SCHEMA_DEFINE_INDEXED(SystemData, name);

struct SectorData {
    std::string id;
    std::string systemId;
    std::string name;
    Vector2 pos;
    uint64_t seed = 0;
    bool generated = false;

    MSGPACK_DEFINE(id, systemId, name, pos, seed, generated);
};

SCHEMA_DEFINE_INDEXED(SectorData, name);
} // namespace Scissio
