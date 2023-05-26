#include "schemas.hpp"
#include "lua.hpp"
#include <sol/sol.hpp>

using namespace Engine;

template <typename T> class DatabaseWrapper;

template <typename T> class DatabaseWrapper {
public:
    explicit DatabaseWrapper(DatabaseOperations& db) : db{db} {
    }

    std::optional<T> find(const std::string& key) {
        return db.find<T>(key);
    }

    T get(const std::string& key) {
        return db.get<T>(key);
    }

    std::optional<T> findForUpdate(const std::string& key) {
        auto ptr = dynamic_cast<DatabaseTransaction*>(&db);
        if (!ptr) {
            throw std::runtime_error("Database find_for_update() function can be called only in transaction");
        }
        return ptr->getForUpdate<T>(key);
    }

    void put(const std::string& key, const T& value) {
        db.put<T>(key, value);
    }

    sol::as_table_t<std::vector<T>> multiGet(const sol::as_table_t<std::vector<std::string>>& keys) {
        return db.multiGet<T>(keys.value());
    }

    bool remove(const std::string& key) {
        return db.remove<T>(key);
    }

    DatabaseIterator<T> seek(const std::string& prefix, const std::variant<std::nullptr_t, std::string>& lowerBound) {
        return db.seek<T>(prefix, lowerBound.index() == 1 ? std::get<std::string>(lowerBound) : "");
    }

    void removeByPrefix(const std::string& key) {
        db.removeByPrefix<T>(key);
    }

    sol::as_table_t<std::vector<T>> seekAll(const std::string& key, const size_t max) {
        return db.seekAll<T>(key, max);
    }

    T update(const std::string& key, const std::function<T(std::optional<T>)>& callback) {
        auto ptr = dynamic_cast<Database*>(&db);
        if (!ptr) {
            throw std::runtime_error("Database update() function can not be called in transaction");
        }
        return ptr->update(key, callback);
    }

    void transaction(const std::function<bool(DatabaseTransaction&)>& callback) {
        auto ptr = dynamic_cast<Database*>(&db);
        if (!ptr) {
            throw std::runtime_error("Database transaction() function can not be called in transaction");
        }
        ptr->transaction(callback);
    }

private:
    DatabaseOperations& db;
};

template <> class DatabaseWrapper<MetaData> {
public:
    explicit DatabaseWrapper(DatabaseOperations& db) : db{db} {
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
        auto ptr = dynamic_cast<DatabaseTransaction*>(&db);
        if (!ptr) {
            throw std::runtime_error("Database find_for_update() function can be called only in transaction");
        }
        auto found = ptr->getForUpdate<MetaData>(key);
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

    bool remove(const std::string& key) {
        return db.remove<MetaData>(key);
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
        auto ptr = dynamic_cast<Database*>(&db);
        if (!ptr) {
            throw std::runtime_error("Database update() function can not be called in transaction");
        }
        const auto res = ptr->update<MetaData>(key, [&](std::optional<MetaData> found) -> MetaData {
            auto res = callback(found.has_value() ? std::optional<MetaDataValue>{found->value} : std::nullopt);
            MetaData v{};
            v.value = res;
            return v;
        });
        return res.value;
    }

    void transaction(const std::function<bool(DatabaseTransaction&)>& callback) {
        auto ptr = dynamic_cast<Database*>(&db);
        if (!ptr) {
            throw std::runtime_error("Database transaction() function can not be called in transaction");
        }
        ptr->transaction(callback);
    }

private:
    DatabaseOperations& db;
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
        using Iterator = DatabaseIterator<T>;
        using Repository = DatabaseWrapper<T>;

        auto& m = lua.root();

        { // Iterator
            auto cls = m.new_usertype<Iterator>(name + "Iterator");
            cls["next"] = &Iterator::next;
            cls["value"] = sol::readonly_property(&Iterator::value);
            cls["key"] = sol::readonly_property(&Iterator::key);
        }

        { // Repository
            auto cls = m.new_usertype<Repository>(name + "Repository");
            cls["find"] = &Repository::find;
            cls["get"] = &Repository::get;
            cls["put"] = &Repository::put;
            cls["multi_get"] = &Repository::multiGet;
            cls["remove"] = &Repository::remove;
            cls["seek"] = &Repository::seek;
            cls["remove_by_prefix"] = &Repository::removeByPrefix;
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

        { // Repository
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

    { // GalaxyData
        auto cls = m.new_usertype<GalaxyData>("GalaxyData", sol::constructors<GalaxyData>{});
        cls["id"] = &GalaxyData::id;
        cls["pos"] = &GalaxyData::pos;
        cls["seed"] = &GalaxyData::seed;
        cls["name"] = &GalaxyData::name;
    }

    { // RegionData
        auto cls = m.new_usertype<RegionData>("RegionData", sol::constructors<RegionData>{});
        cls["id"] = &RegionData::id;
        cls["pos"] = &RegionData::pos;
        cls["name"] = &RegionData::name;
    }

    { // FactionData
        auto cls = m.new_usertype<FactionData>("FactionData", sol::constructors<FactionData>{});
        cls["id"] = &FactionData::id;
        cls["name"] = &FactionData::name;
        cls["color"] = &FactionData::color;
        cls["home_galaxy_id"] = &FactionData::homeGalaxyId;
        cls["home_system_id"] = &FactionData::homeSystemId;
    }

    { // SystemData
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

    { // SectorData
        auto cls = m.new_usertype<SectorData>("SectorData", sol::constructors<SectorData>{});
        cls["id"] = &SectorData::id;
        cls["name"] = &SectorData::name;
        cls["galaxy_id"] = &SectorData::galaxyId;
        cls["system_id"] = &SectorData::systemId;
        cls["seed"] = &SectorData::seed;
        cls["pos"] = &SectorData::pos;
    }

    { // PlanetData
        auto cls = m.new_usertype<PlanetData>("PlanetData", sol::constructors<PlanetData>{});
        cls["id"] = &PlanetData::id;
        cls["name"] = &PlanetData::name;
        cls["galaxy_id"] = &PlanetData::galaxyId;
        cls["system_id"] = &PlanetData::systemId;
        cls["seed"] = &PlanetData::seed;
        cls["pos"] = &PlanetData::pos;
        cls["parent"] = &PlanetData::parent;
        cls["type"] = &PlanetData::type;
    }

    { // PlanetData
        auto cls = m.new_usertype<PlayerLocationData>("PlayerLocationData", sol::constructors<PlayerLocationData>{});
        cls["galaxy_id"] = &PlayerLocationData::galaxyId;
        cls["system_id"] = &PlayerLocationData::systemId;
        cls["sector_id"] = &PlayerLocationData::sectorId;
    }

    { // Transaction
        auto cls = m.new_usertype<DatabaseTransaction>("DatabaseTransaction");
        BindSchemaHelper<GalaxyData>::bind(lua, cls, "galaxies", "GalaxyData");
        BindSchemaHelper<RegionData>::bind(lua, cls, "regions", "RegionData");
        BindSchemaHelper<FactionData>::bind(lua, cls, "factions", "FactionData");
        BindSchemaHelper<SystemData>::bind(lua, cls, "systems", "SystemData");
        BindSchemaHelper<SectorData>::bind(lua, cls, "sectors", "SectorData");
        BindSchemaHelper<PlanetData>::bind(lua, cls, "planets", "PlanetData");
        BindSchemaHelper<PlayerLocationData>::bind(lua, cls, "player_locations", "PlayerLocationData");
        BindSchemaHelper<MetaData>::bind(lua, cls, "metadata", "MetaData");
    }

    { // Database
        auto cls = m.new_usertype<Database>("Database");
        bindRepository<GalaxyData>(cls, "galaxies");
        bindRepository<RegionData>(cls, "regions");
        bindRepository<FactionData>(cls, "factions");
        bindRepository<SystemData>(cls, "systems");
        bindRepository<SectorData>(cls, "sectors");
        bindRepository<PlanetData>(cls, "planets");
        bindRepository<PlayerLocationData>(cls, "player_locations");
        bindRepository<MetaData>(cls, "metadata");
    }
}
