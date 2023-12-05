#include "../Server.hpp"
#include "Bindings.hpp"

using namespace Engine;

static void bindServer(sol::table& m) {
    /**
     * @module engine
     */
    /**
     * @class Server
     * A class that represents server operations
     */
    auto cls = m.new_usertype<Server>("Server");
    /**
     * @function Server:move_player_to_sector
     * Moves a player to sector and starts the sector if it is not running
     * @param player_id The ID of the player to move
     * @praam sector_id The ID of the sector
     */
    cls["move_player_to_sector"] = &Server::movePlayerToSector;
    /**
     * @function Server:add_sector_type
     * Adds a new sector type, or overwrites an existing sector type, for the galaxy generator.
     * @param name Name of the sector type
     * @param type An instance of SectorType
     */
    cls["add_sector_type"] = &Server::addSectorType;
}

LUA_BINDINGS(bindServer);

static void bindSession(sol::table& m) {
    auto cls = m.new_usertype<Session>("Session");
}

LUA_BINDINGS(bindSession);

static void bindSpawner(sol::table& m) {
    /**
     * @module engine
     */

    /**
     * @class Spawner
     * A class that represents a weighted entity
     */
    /**
     * @function Spawner.new
     * Default constructor that initializes the weighted entity with no entity and the default weight.
     */
    /**
     * @function Spawner.new
     * Parametrized constructor that initializes the weighted entity
     * @param entity string The name of the entity
     * @param weight number The numerical weight of the entity
     * @param count number How many of this entity we should spawn
     */
    auto cls = m.new_usertype<Spawner>("Spawner", sol::constructors<Spawner(), Spawner(std::string, float, int)>());
    /**
     * @field Spawner.entity
     * The string name of the entity (not the Lua package name!)
     * @type string
     */
    cls["entity"] = &Spawner::entity;
    /**
     * @field Spawner.weight
     * The weight of the entity (default value is 1.0)
     * @type number
     */
    cls["weight"] = &Spawner::weight;
    /**
     * @field Spawner.count
     * How many of this entity we should spawn.
     * @type number
     */
    cls["count"] = &Spawner::count;
}

LUA_BINDINGS(bindSpawner);

static void bindSectorType(sol::table& m) {
    /**
     * @module engine
     */
    /**
     * @class SectorType
     * A class that represents a some specific sector type with conditions
     */
    /**
     * @function SectorType.new
     * Default constructor
     */
    auto cls = m.new_usertype<SectorType>("SectorType", sol::constructors<SectorType()>());
    /**
     * @field SectorType.map_icon
     * The image icon of this sector to show on the system map within the game
     * @type Image
     */
    cls["map_icon"] = &SectorType::mapIcon;
    /**
     * @field SectorType.weight
     * The weight of this type, when considering between many types to choose from
     * when generating the sectors within a system.
     * @type number
     */
    cls["weight"] = &SectorType::weight;
    /**
     * @field SectorType.min_count
     * The minimum number of sectors with this type that should exist in a single system.
     * Default value is 1.
     * @type number
     */
    cls["min_count"] = &SectorType::minCount;
    /**
     * @field SectorType.max_count
     * The maximum number of sectors with this type that can exist in a single system.
     * Default value is 1.
     * @type number
     */
    cls["max_count"] = &SectorType::maxCount;
    /**
     * @field SectorType.entities
     * A list of entities (weighted entity) to spawn in this sector.
     * @type <WeightedEntity>[] A list of WeightedEntity items
     */
    cls["entities"] = &SectorType::entities;
    /**
     * @field SectorType.conditions
     * A list of sector conditions that should apply when considering this sector to generate.
     * @type <SectorCondition>[] A list of SectorCondition items
     */
    cls["conditions"] = &SectorType::conditions;
}

LUA_BINDINGS(bindSectorType);

static void bindSectorCondition(sol::table& m) {
    /**
     * @module engine
     */
    /**
     * @class SectorCondition
     * A class that represents a weighted entity
     */
    /**
     * @function SectorCondition.new
     * Default constructor that initializes the weighted entity with no entity and the default weight.
     */
    /**
     * @function SectorCondition.new
     * Parametrized constructor that initializes the weighted entity
     * @param name string The name of the condition
     * @param value any The value of such condition
     */
    auto cls = m.new_usertype<SectorCondition>(
        "WeightedEntity", sol::constructors<SectorCondition(), SectorCondition(std::string, SectorCondition::Value)>());
    /**
     * @field SectorCondition.name
     * The name of the condition
     * @type string
     */
    cls["name"] = &SectorCondition::name;
    /**
     * @field SectorCondition.value
     * The value of such condition
     * @type any
     */
    cls["value"] = &SectorCondition::value;
}

LUA_BINDINGS(bindSectorCondition);
