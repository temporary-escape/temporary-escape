#include "SectorType.hpp"
#include "Lua.hpp"
#include <sol/sol.hpp>

using namespace Engine;

void Spawner::bind(Lua& lua) {
    /**
     * @module engine
     */
    auto& m = lua.root();

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

void SectorType::bind(Lua& lua) {
    /**
     * @module engine
     */
    auto& m = lua.root();

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

void SectorCondition::bind(Lua& lua) {
    /**
     * @module engine
     */
    auto& m = lua.root();

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

bool SectorType::checkConditions(Rng& rng, const GalaxyData& galaxy, const SystemData& system,
                                 const std::vector<PlanetData>& planets) const {
    const auto testWeight = randomReal<float>(rng, 0.0f, 1.0f);
    if (testWeight > weight) {
        return false;
    }

    return true;
}
