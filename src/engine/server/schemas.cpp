#include "schemas.hpp"
#include "lua.hpp"
#include <sol/sol.hpp>

using namespace Engine;

template <typename T> class DatabaseWrapper;

template <typename T> class DatabaseWrapper {
public:
    explicit DatabaseWrapper(Database& db) : db{db} {
    }

    std::optional<T> find(const std::string& key) {
        return db.find<T>(key);
    }

    T get(const std::string& key) {
        return db.get<T>(key);
    }

    void put(const std::string& key, const T& value) {
        db.put<T>(key, value);
    }

    sol::as_table_t<std::vector<T>> multiGet(const sol::as_table_t<std::vector<std::string>>& keys) {
        return db.multiGet<T>(keys.value());
    }

    void remove(const std::string& key) {
        db.remove<T>(key);
    }

    Database::Iterator<T> seek(const std::string& prefix, const std::variant<std::nullptr_t, std::string>& lowerBound) {
        return db.seek<T>(prefix, lowerBound.index() == 1 ? std::get<std::string>(lowerBound) : "");
    }

    void removeByPrefix(const std::string& key) {
        db.removeByPrefix<T>(key);
    }

    sol::as_table_t<std::vector<T>> seekAll(const std::string& key, const size_t max) {
        return db.seekAll<T>(key, max);
    }

    T update(const std::string& key, const std::function<T(std::optional<T>)>& callback) {
        return db.update(key, callback);
    }

    void transaction(const std::function<bool(Database&)>& callback) {
        db.transaction(callback);
    }

private:
    Database& db;
};

template <> class DatabaseWrapper<MetaData> {
public:
    explicit DatabaseWrapper(Database& db) : db{db} {
    }

    std::optional<MetaDataValue> find(const std::string& key) {
        auto found = db.find<MetaData>(key);
        if (!found) {
            return {};
        }
        return found->value;
    }

    MetaDataValue get(const std::string& key) {
        return db.get<MetaData>(key).value;
    }

    std::optional<MetaDataValue> findForUpdate(const std::string& key) {
        auto found = db.find<MetaData>(key);
        if (!found) {
            return {};
        }
        return found->value;
    }

    void put(const std::string& key, const MetaDataValue& value) {
        MetaData v{};
        v.value = value;
        db.put<MetaData>(key, v);
    }

    sol::as_table_t<std::vector<MetaDataValue>> multiGet(const sol::as_table_t<std::vector<std::string>>& keys) {
        const auto found = db.multiGet<MetaData>(keys.value());
        std::vector<MetaDataValue> results;
        results.resize(found.size());
        for (size_t i = 0; i < found.size(); i++) {
            results[i] = found[i].value;
        }
        return results;
    }

    void remove(const std::string& key) {
        db.remove<MetaData>(key);
    }

    void removeByPrefix(const std::string& key) {
        db.removeByPrefix<MetaData>(key);
    }

    sol::as_table_t<std::vector<MetaDataValue>> seekAll(const std::string& key, const size_t max) {
        const auto found = db.seekAll<MetaData>(key, max);
        std::vector<MetaDataValue> results;
        results.resize(found.size());
        for (size_t i = 0; i < found.size(); i++) {
            results[i] = found[i].value;
        }
        return results;
    }

    MetaDataValue update(const std::string& key,
                         const std::function<MetaDataValue(std::optional<MetaDataValue>)>& callback) {
        const auto res = db.update<MetaData>(key, [&](std::optional<MetaData> found) -> MetaData {
            auto res = callback(found.has_value() ? std::optional<MetaDataValue>{found->value} : std::nullopt);
            MetaData v{};
            v.value = res;
            return v;
        });
        return res.value;
    }

    void transaction(const std::function<bool(Database&)>& callback) {
        auto ptr = dynamic_cast<Database*>(&db);
        if (!ptr) {
            throw std::runtime_error("Database transaction() function can not be called in transaction");
        }
        ptr->transaction(callback);
    }

private:
    Database& db;
};

template <typename T, typename C> static DatabaseWrapper<T> getWrapper(C& db) {
    return DatabaseWrapper<T>{db};
}

template <typename T, typename C> static void bindRepository(sol::usertype<C>& klass, const std::string& field) {
    klass[field] = sol::readonly_property(&getWrapper<T, C>);
}

template <typename T> struct BindSchemaHelper {
    template <typename C>
    static void bind(Lua& lua, sol::usertype<C>& klass, const std::string& field, const std::string& name) {
        using Iterator = Database::Iterator<T>;
        using Repository = DatabaseWrapper<T>;

        auto& m = lua.root();

        /**
         * @module engine
         */

        {
            /**
             * @class Iterator<T>
             * An iterator that is used by the seek() function from the Repository class.
             * This iterator can be used to iterate through set of values, one at a time, by some prefix.
             * @see Repository<T>
             */
            auto cls = m.new_usertype<Iterator>(name + "Iterator");
            /**
             * @function Iterator<T>:next
             * Retrieves the next value
             * @warning This function must be called first before trying to read the value or the key.
             * @return bool True if there is some value to read or false if this is end of the seek
             */
            cls["next"] = &Iterator::next;
            /**
             * @field Iterator<T>.value
             * The current value that the iterator points to. You must first call
             * next() before using this field, otherwise this raises an error.
             * @readonly
             * @type T
             */
            cls["value"] = sol::readonly_property(&Iterator::value);
            /**
             * @field Iterator<T>.value
             * The current key that the iterator points to. You must first call
             * next() before using this field, otherwise this raises an error.
             * @readonly
             * @type string
             */
            cls["key"] = sol::readonly_property(&Iterator::key);
        }

        { // Repository
            /**
             * @class Repository<T>
             * Repository of some specific database type
             * @code
             * local engine = require("engine")
             * local db = engine.get_database()
             *
             * local faction_terran = db.factions:find("faction_terran")
             * if faction_terran == nil then
             *     error("Faction was not found")
             * else
             *     print(string.format("Found faction: %s", faction_terran.name))
             * end
             * @endcode
             */
            auto cls = m.new_usertype<Repository>(name + "Repository");
            /**
             * @function Repository<T>:find
             * Finds the value given some key
             * @param key string The key of the value
             * @return T|nil The value or nil if not found
             */
            cls["find"] = &Repository::find;
            /**
             * @function Repository<T>:get
             * Similar to the find function but raises an error if the key is not found
             * @param key string The key of the value
             * @return T The value under that specific key
             */
            cls["get"] = &Repository::get;
            /**
             * @function Repository<T>:put
             * Puts or overwrites a value with some key
             * @param key string The key of the value
             * @param value T The value to store
             */
            cls["put"] = &Repository::put;
            /**
             * @function Repository<T>:multi_get
             * Returns values as a list using a list of keys
             * @param keys <string>[] A list of keys to retrieve
             * @return <T>[] A list (table) of results or an empty list
             */
            cls["multi_get"] = &Repository::multiGet;
            /**
             * @function Repository<T>:remove
             * Removes a key from the database
             * @param key string The key to remove
             * @return bool True if removed or false otherwise
             */
            cls["remove"] = &Repository::remove;
            /**
             * @function Repository<T>:seek
             * Seeks a value by some prefix and an optional lower bound
             * @param prefix string The prefix to seek through
             * @return Iterator<T> An instance of an iterator
             * @see Iterator<T>
             */
            cls["seek"] = &Repository::seek;
            /**
             * @function Repository<T>:remove_by_prefix
             * Removes all values by some prefix from the database
             * @param prefix string The prefix of keys to remove
             */
            cls["remove_by_prefix"] = &Repository::removeByPrefix;
            /**
             * @function Repository<T>:seek_all
             * Returns all values as a list by some prefix from the database
             * @param prefix string The prefix of keys to retrieve
             * @param max integer|nil Maximum number of items to get (set to nil to get all results)
             * @return <T>[] A list (table) of results or an empty list
             */
            cls["seek_all"] = &Repository::seekAll;
            cls["update"] = &Repository::update;
            cls["transaction"] = &Repository::transaction;
        }

        bindRepository<T, C>(klass, field);
    }
};

template <> struct BindSchemaHelper<MetaData> {
    template <typename C>
    static void bind(Lua& lua, sol::usertype<C>& klass, const std::string& field, const std::string& name) {
        using Repository = DatabaseWrapper<MetaData>;

        auto& m = lua.root();

        {
            auto cls = m.new_usertype<Repository>(name + "Repository");
            cls["find"] = &Repository::find;
            cls["get"] = &Repository::get;
            cls["put"] = &Repository::put;
            cls["multi_get"] = &Repository::multiGet;
            cls["remove"] = &Repository::remove;
            cls["remove_by_prefix"] = &Repository::removeByPrefix;
            cls["seek_all"] = &Repository::seekAll;
            cls["update"] = &Repository::update;
            cls["transaction"] = &Repository::transaction;
        }

        bindRepository<MetaData, C>(klass, field);
    }
};

void Engine::bindSchemas(Lua& lua) {
    auto& m = lua.root();
    {
        /**
         * @class GalaxyData
         * Database type that represents basic galaxy information such us its name
         */
        /**
         * @function GalaxyData.new
         * Default constructor
         * @type string
         */
        auto cls = m.new_usertype<GalaxyData>("GalaxyData", sol::constructors<GalaxyData>{});
        /**
         * @field GalaxyData.id
         * A unique UUID of the galaxy
         * @type string
         */
        cls["id"] = &GalaxyData::id;
        /**
         * @field GalaxyData.pos
         * The position within the system
         * @type Vector2
         */
        cls["pos"] = &GalaxyData::pos;
        /**
         * @field GalaxyData.seed
         * A seed that is specific to this galaxy
         * @type integer
         */
        cls["seed"] = &GalaxyData::seed;
        /**
         * @field GalaxyData.name
         * Name of the galaxy
         * @type string
         */
        cls["name"] = &GalaxyData::name;
    }

    {
        /**
         * @class RegionData
         * Database type that represents region data within a galaxy
         */
        /**
         * @function RegionData.new
         * Default constructor
         * @type string
         */
        auto cls = m.new_usertype<RegionData>("RegionData", sol::constructors<RegionData>{});
        cls["id"] = &RegionData::id;
        cls["pos"] = &RegionData::pos;
        cls["name"] = &RegionData::name;
    }

    {
        /**
         * @class FactionData
         * Database type that represents some faction
         */
        /**
         * @function FactionData.new
         * Default constructor
         * @type string
         */
        auto cls = m.new_usertype<FactionData>("FactionData", sol::constructors<FactionData>{});
        cls["id"] = &FactionData::id;
        cls["name"] = &FactionData::name;
        cls["color"] = &FactionData::color;
        cls["home_galaxy_id"] = &FactionData::homeGalaxyId;
        cls["home_system_id"] = &FactionData::homeSystemId;
    }

    {
        /**
         * @class SystemData
         * Database type that represents some system within a galaxy that also belongs to a region
         */
        /**
         * @function SystemData.new
         * Default constructor
         * @type string
         */
        auto cls = m.new_usertype<SystemData>("SystemData", sol::constructors<SystemData>{});
        cls["id"] = &SystemData::id;
        cls["name"] = &SystemData::name;
        cls["galaxy_id"] = &SystemData::galaxyId;
        cls["region_id"] = &SystemData::regionId;
        cls["faction_id"] = &SystemData::factionId;
        cls["seed"] = &SystemData::seed;
        cls["pos"] = &SystemData::pos;
        cls["connections"] = &SystemData::connections;
    }

    {
        /**
         * @class SectorData
         * Database type that represents some sector within a system
         */
        /**
         * @function SectorData.new
         * Default constructor
         * @type string
         */
        auto cls = m.new_usertype<SectorData>("SectorData", sol::constructors<SectorData>{});
        cls["id"] = &SectorData::id;
        cls["name"] = &SectorData::name;
        cls["galaxy_id"] = &SectorData::galaxyId;
        cls["system_id"] = &SectorData::systemId;
        cls["seed"] = &SectorData::seed;
        cls["pos"] = &SectorData::pos;
        cls["icon"] = &SectorData::icon;
        cls["type"] = &SectorData::type;
    }

    {
        /**
         * @class PlanetData
         * Database type that represents some planet within a system
         */
        /**
         * @function PlanetData.new
         * Default constructor
         * @type string
         */
        auto cls = m.new_usertype<PlanetData>("PlanetData", sol::constructors<PlanetData>{});
        cls["id"] = &PlanetData::id;
        cls["name"] = &PlanetData::name;
        cls["galaxy_id"] = &PlanetData::galaxyId;
        cls["system_id"] = &PlanetData::systemId;
        cls["seed"] = &PlanetData::seed;
        cls["pos"] = &PlanetData::pos;
        cls["parent_id"] = &PlanetData::parentId;
        cls["type"] = &PlanetData::type;
        cls["radius"] = &PlanetData::radius;
    }

    {
        /**
         * @class PlayerLocationData
         * Database type that specifies the location of the player within the universe
         */
        /**
         * @function PlayerLocationData.new
         * Default constructor
         * @type string
         */
        auto cls = m.new_usertype<PlayerLocationData>("PlayerLocationData", sol::constructors<PlayerLocationData>{});
        cls["galaxy_id"] = &PlayerLocationData::galaxyId;
        cls["system_id"] = &PlayerLocationData::systemId;
        cls["sector_id"] = &PlayerLocationData::sectorId;
    }

    {
        /**
         * @class StartingLocationData
         * Database type that specifies the starting location for new players.
         */
        auto cls =
            m.new_usertype<StartingLocationData>("StartingLocationData", sol::constructors<StartingLocationData>{});
        cls["galaxy_id"] = &StartingLocationData::galaxyId;
        cls["system_id"] = &StartingLocationData::systemId;
        cls["sector_id"] = &StartingLocationData::sectorId;
    }

    {
        /**
         * @class Database
         * Database class that can be used to access all of the data structures.
         */
        auto cls = m.new_usertype<Database>("Database");
        /**
         * @field Database.galaxies
         * @type Repository<GalaxyData>
         * @readonly
         */
        BindSchemaHelper<GalaxyData>::bind(lua, cls, "galaxies", "GalaxyData");
        /**
         * @field Database.regions
         * @type Repository<RegionData>
         * @readonly
         */
        BindSchemaHelper<RegionData>::bind(lua, cls, "regions", "RegionData");
        /**
         * @field Database.factions
         * @type Repository<FactionData>
         * @readonly
         */
        BindSchemaHelper<FactionData>::bind(lua, cls, "factions", "FactionData");
        /**
         * @field Database.systems
         * @type Repository<SystemData>
         * @readonly
         */
        BindSchemaHelper<SystemData>::bind(lua, cls, "systems", "SystemData");
        /**
         * @field Database.sectors
         * @type Repository<SectorData>
         * @readonly
         */
        BindSchemaHelper<SectorData>::bind(lua, cls, "sectors", "SectorData");
        /**
         * @field Database.planets
         * @type Repository<PlanetData>
         * @readonly
         */
        BindSchemaHelper<PlanetData>::bind(lua, cls, "planets", "PlanetData");
        /**
         * @field Database.player_locations
         * @type Repository<PlayerLocationData>
         * @readonly
         */
        BindSchemaHelper<PlayerLocationData>::bind(lua, cls, "player_locations", "PlayerLocationData");
        /**
         * @field Database.metadata
         * @type Repository<MetaData>
         * @readonly
         */
        BindSchemaHelper<MetaData>::bind(lua, cls, "metadata", "MetaData");
        BindSchemaHelper<StartingLocationData>::bind(lua, cls, "starting_locations", "StartingLocationData");
    }
}
