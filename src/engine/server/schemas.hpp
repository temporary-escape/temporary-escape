#pragma once

#include "../assets/assets_manager.hpp"
#include "../config.hpp"
#include "../database/database.hpp"
#include "../math/vector.hpp"

namespace Engine {
using MetaDataValue = std::variant<bool, std::string, int64_t, double>;

struct MetaData {
    MetaDataValue value;

    MSGPACK_DEFINE_MAP(value);
};

SCHEMA_DEFINE(MetaData);

struct PlayerData {
    std::string id;
    uint64_t secret;
    std::string name;
    bool admin;

    MSGPACK_DEFINE_MAP(id, secret, name, admin);
};

SCHEMA_DEFINE(PlayerData);
SCHEMA_INDEXES(PlayerData, secret);

struct PlayerLocationData {
    std::string galaxyId;
    std::string systemId;
    std::string sectorId;

    MSGPACK_DEFINE_MAP(galaxyId, systemId, sectorId);
};

SCHEMA_DEFINE(PlayerLocationData);

struct FactionData {
    std::string id;
    std::string name;
    float color{0.0f};
    std::string homeGalaxyId;
    std::string homeSystemId;

    MSGPACK_DEFINE_MAP(id, name, color, homeGalaxyId, homeSystemId);
};

SCHEMA_DEFINE(FactionData);
SCHEMA_INDEXES(FactionData, name);

struct GalaxyData {
    std::string id;
    std::string name;
    Vector2 pos;
    uint64_t seed = 0;

    MSGPACK_DEFINE_MAP(id, name, pos, seed);
};

SCHEMA_DEFINE(GalaxyData);
SCHEMA_INDEXES(GalaxyData, name);

struct RegionData {
    std::string id;
    std::string name;
    Vector2 pos;

    MSGPACK_DEFINE_MAP(id, name, pos);
};

SCHEMA_DEFINE(RegionData);
SCHEMA_INDEXES(RegionData, name);

struct SectorData {
    std::string id;
    std::string galaxyId;
    std::string systemId;
    std::string name;
    Vector2 pos{0.0f, 0.0f};
    uint64_t seed{0};
    ImagePtr icon{nullptr};
    std::string luaTemplate;

    MSGPACK_DEFINE_MAP(id, galaxyId, systemId, name, pos, seed, icon, luaTemplate);
};

SCHEMA_DEFINE(SectorData);
SCHEMA_INDEXES(SectorData, name);

struct StartingLocationData {
    std::string galaxyId;
    std::string systemId;
    std::string sectorId;

    MSGPACK_DEFINE_MAP(galaxyId, systemId, sectorId);
};

SCHEMA_DEFINE(StartingLocationData);

struct SystemData {
    std::string id;
    std::string galaxyId;
    std::string regionId;
    std::string name;
    std::optional<std::string> factionId;
    Vector2 pos;
    uint64_t seed{0};
    std::vector<std::string> connections;

    MSGPACK_DEFINE_MAP(id, galaxyId, regionId, name, factionId, pos, seed, connections);
};

SCHEMA_DEFINE(SystemData);
SCHEMA_INDEXES(SystemData, name);

struct PlanetData {
    std::string id;
    std::string galaxyId;
    std::string systemId;
    std::string name;
    std::optional<std::string> parentId;
    PlanetTypePtr type;
    Vector2 pos;
    uint64_t seed{0};
    float radius{0.5f};

    MSGPACK_DEFINE_MAP(id, galaxyId, systemId, name, parentId, type, pos, seed, radius);
};

SCHEMA_DEFINE(PlanetData);

ENGINE_API void bindSchemas(Lua& lua);
} // namespace Engine
