#pragma once

#include "../Assets/AssetManager.hpp"
#include "../Assets/Model.hpp"
#include "../Utils/Database.hpp"
#include "../Utils/Msgpack.hpp"
#include "../Utils/Repository.hpp"
#include "../Utils/Xml.hpp"

#define DB_SCHEMA_ENUM(E, P)                                                                                           \
    DEFINE_ENUM_STR(P);                                                                                                \
    template <> struct Database::Helper<E> {                                                                           \
        static void set(sqlite3_stmt* stmt, const int idx, const E& value) {                                           \
            Helper<std::string>::set(stmt, idx, enumToStr(value));                                                     \
        }                                                                                                              \
        static E get(sqlite3_stmt* stmt, const int idx) {                                                              \
            const auto name = Helper<std::string>::get(stmt, idx);                                                     \
            return strToEnum<E>(name);                                                                                 \
        }                                                                                                              \
    };

namespace Scissio {
template <> struct Database::Helper<ModelPtr> {
    static void set(sqlite3_stmt* stmt, const int idx, const ModelPtr& value) {
        Helper<std::string>::set(stmt, idx, value->getName());
    }

    static ModelPtr get(sqlite3_stmt* stmt, const int idx) {
        const auto name = Helper<std::string>::get(stmt, idx);
        return AssetManager::singleton().findOrNull<Model>(name);
    }
};

template <> struct Database::Helper<std::vector<ModelPtr>> {
    static void set(sqlite3_stmt* stmt, const int idx, const std::vector<ModelPtr>& value) {
        std::stringstream ss;
        auto first = true;
        for (const auto& e : value) {
            if (!first) {
                ss << ",";
            }
            ss << e->getName();
            first = false;
        }
        Helper<std::string>::set(stmt, idx, ss.str());
    }

    static std::vector<ModelPtr> get(sqlite3_stmt* stmt, const int idx) {
        const auto names = split(Helper<std::string>::get(stmt, idx), ",");
        std::vector<ModelPtr> res;
        for (const auto& name : names) {
            auto model = AssetManager::singleton().findOrNull<Model>(name);
            if (model) {
                res.push_back(std::move(model));
            }
        }
        return res;
    }
};

enum class ItemType {
    None = 0,
    Goods,
    Resource,
    Currency,
    Blueprint,
};

DB_SCHEMA_ENUM(ItemType, EnumPairs<ItemType>({
                             {ItemType::None, "None"},
                             {ItemType::Goods, "Goods"},
                             {ItemType::Resource, "Resource"},
                             {ItemType::Currency, "Currency"},
                             {ItemType::Blueprint, "Blueprint"},
                         }));

struct Block {
    uint64_t id{0};
    std::string key;
    std::string title;
    int tier{0};
    std::string description;
    ModelPtr model;
    std::string category;

    DB_TABLE_NAME("Block");

    DB_SCHEMA({
        SchemaField("id").integer().primary().autoInc(),
        SchemaField("key").text().nonNull().unique(),
        SchemaField("title").text().nonNull(),
        SchemaField("tier").integer().nonNull(),
        SchemaField("description").text().nonNull(),
        SchemaField("model").text().nonNull(),
        SchemaField("category").text().nonNull(),
    });

    DB_BIND(key, title, tier, description, model, category);

    static void convert(const Xml::Node& n, Block& v);
};

struct BlockDto {
    std::string key;
    std::string title;
    int tier{0};
    std::string description;
    ModelPtr model;
    std::string category;

    MSGPACK_DEFINE_ARRAY(key, title, tier, description, model, category);

    static BlockDto from(const Block& block) {
        BlockDto dto{};
        dto.key = block.key;
        dto.title = block.title;
        dto.tier = block.tier;
        dto.description = block.description;
        dto.model = block.model;
        dto.category = block.category;
        return dto;
    }
};

struct Asteroid {
    uint64_t id{0};
    std::string key;
    std::string title;
    std::string description;
    float rarity{0.0f};
    float distMin{0.0f};
    float distMax{0.0f};
    std::vector<ModelPtr> models;

    DB_TABLE_NAME("Asteroid");

    DB_SCHEMA({
        SchemaField("id").integer().primary().autoInc(),
        SchemaField("key").text().nonNull().unique(),
        SchemaField("title").text().nonNull(),
        SchemaField("description").text().nonNull(),
        SchemaField("rarity").real().nonNull(),
        SchemaField("distMin").real().nonNull(),
        SchemaField("distMax").real().nonNull(),
        SchemaField("models").text().nonNull(),
    });

    DB_BIND(key, title, description, rarity, distMin, distMax, models);

    static void convert(const Xml::Node& n, Asteroid& v);
};

struct Item {
    uint64_t id{0};
    std::string key;
    std::string title;
    std::string description;
    ItemType type;

    DB_TABLE_NAME("Item");

    DB_SCHEMA({
        SchemaField("id").integer().primary().autoInc(),
        SchemaField("key").text().nonNull().unique(),
        SchemaField("title").text().nonNull(),
        SchemaField("description").text().nonNull(),
        SchemaField("type").text().nonNull(),
    });

    DB_BIND(key, title, description, type);
};

struct Player {
    uint64_t id{0};
    std::string name;
    bool admin{false};
    std::optional<uint64_t> lastLogin;

    DB_TABLE_NAME("Player");

    DB_SCHEMA({
        SchemaField("id").integer().primary().autoInc(),
        SchemaField("name").text().nonNull().indexed(),
        SchemaField("admin").boolean().nonNull().defval("0"),
        SchemaField("lastLogin").integer(),
    });

    DB_BIND(name, admin, lastLogin);
};

struct PlayerBlock {
    uint64_t id{0};
    uint64_t playerId{0};
    uint64_t blockId{0};

    DB_TABLE_NAME("PlayerBlock");

    DB_SCHEMA({
        SchemaField("id").integer().primary().autoInc(),
        SchemaField("playerId").integer().references<Player>("id").nonNull().indexed(),
        SchemaField("blockId").integer().references<Block>("id").nonNull(),
    });

    DB_BIND(playerId, blockId);
};

struct Faction {
    uint64_t id{0};
    std::string name;

    DB_TABLE_NAME("Faction");

    DB_SCHEMA({
        SchemaField("id").integer().primary().autoInc(),
        SchemaField("name").text().nonNull().unique(),
    });

    DB_BIND(name);
};

struct Galaxy {
    uint64_t id{0};
    std::string name;
    uint64_t seed{0};

    DB_TABLE_NAME("Galaxy");

    DB_SCHEMA({
        SchemaField("id").integer().primary().autoInc(),
        SchemaField("name").text().nonNull().unique(),
        SchemaField("seed").integer().nonNull(),
    });

    DB_BIND(name, seed);
};

struct Region {
    uint64_t id{0};
    std::string name;
    uint64_t galaxyId{0};
    float posX{0.0f};
    float posY{0.0f};
    float hue{0.0f};

    DB_TABLE_NAME("Region");

    DB_SCHEMA({
        SchemaField("id").integer().primary().autoInc(),
        SchemaField("name").text().nonNull().indexed(),
        SchemaField("galaxyId").integer().references<Galaxy>("id").nonNull(),
        SchemaField("posX").real().nonNull(),
        SchemaField("posY").real().nonNull(),
        SchemaField("hue").real().nonNull(),
    });

    DB_BIND(name, galaxyId, posX, posY, hue);
};

struct RegionDto {
    uint64_t id{0};
    std::string name;
    Vector2 pos;
    float hue;

    MSGPACK_DEFINE_ARRAY(id, name, pos, hue);

    static RegionDto from(const Region& region) {
        RegionDto dto;
        dto.id = region.id;
        dto.name = region.name;
        dto.pos = Vector2{region.posX, region.posY};
        dto.hue = region.hue;
        return dto;
    }
};

struct System {
    uint64_t id{0};
    std::string name;
    uint64_t galaxyId{0};
    uint64_t regionId{0};
    float posX{0.0f};
    float posY{0.0f};
    uint64_t seed{0};
    std::optional<uint64_t> factionId{std::nullopt};

    DB_TABLE_NAME("System");

    DB_SCHEMA({
        SchemaField("id").integer().primary().autoInc(),
        SchemaField("name").text().nonNull().indexed(),
        SchemaField("regionId").integer().references<Region>("id").nonNull(),
        SchemaField("galaxyId").integer().references<Galaxy>("id").nonNull(),
        SchemaField("posX").real().nonNull(),
        SchemaField("posY").real().nonNull(),
        SchemaField("seed").integer().nonNull(),
        SchemaField("factionId").integer().references<Faction>("id").defval("NULL"),
    });

    DB_BIND(name, galaxyId, regionId, posX, posY, seed, factionId);
};

struct SystemDto {
    uint64_t id{0};
    std::string name;
    Vector2 pos;
    uint64_t regionId{0};
    std::vector<uint64_t> links;

    MSGPACK_DEFINE_ARRAY(id, name, pos, regionId, links);

    static SystemDto from(const System& system, std::vector<uint64_t> links) {
        SystemDto dto;
        dto.id = system.id;
        dto.name = system.name;
        dto.pos = Vector2{system.posX, system.posY};
        dto.regionId = system.regionId;
        dto.links = std::move(links);

        return dto;
    }
};

struct SystemLink {
    uint64_t id{0};
    uint64_t sourceId{0};
    uint64_t destinationId{0};

    DB_TABLE_NAME("SystemLink");

    DB_SCHEMA({
        SchemaField("id").integer().primary().autoInc(),
        SchemaField("sourceId").integer().references<System>("id").nonNull(),
        SchemaField("destinationId").integer().references<System>("id").nonNull(),
    });

    DB_BIND(sourceId, destinationId);
};

struct Sector {
    uint64_t id{0};
    std::string name;
    uint64_t systemId{0};
    float posX{0.0f};
    float posY{0.0f};
    uint64_t seed{0};

    DB_TABLE_NAME("Sector");

    DB_SCHEMA({
        SchemaField("id").integer().primary().autoInc(),
        SchemaField("name").text().nonNull().unique(),
        SchemaField("systemId").integer().references<System>("id").nonNull(),
        SchemaField("posX").real().nonNull(),
        SchemaField("posY").real().nonNull(),
        SchemaField("seed").integer().nonNull(),
    });

    DB_BIND(name, systemId, posX, posY, seed);
};

struct SectorDto {
    uint64_t id{0};
    std::string name;
    Vector2 pos;

    MSGPACK_DEFINE_ARRAY(id, name, pos);

    static SectorDto from(const Sector& sector) {
        SectorDto dto;
        dto.id = sector.id;
        dto.name = sector.name;
        dto.pos = Vector2{sector.posX, sector.posY};
        return dto;
    }
};

struct PlayerLocation {
    uint64_t id{0};
    uint64_t playerId{0};
    uint64_t sectorId{0};

    DB_TABLE_NAME("PlayerLocation");

    DB_SCHEMA({
        SchemaField("id").integer().primary().autoInc(),
        SchemaField("playerId").integer().references<Player>("id").nonNull().unique(),
        SchemaField("sectorId").integer().references<Sector>("id").nonNull(),
    });

    DB_BIND(playerId, sectorId);
};

SCISSIO_API extern void createSchemas(Database& db);
} // namespace Scissio

MSGPACK_ADD_ENUM(Scissio::ItemType);
