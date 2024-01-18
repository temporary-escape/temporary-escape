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
}

LUA_BINDINGS(bindServer);

static void bindSession(sol::table& m) {
    auto cls = m.new_usertype<Session>("Session");
}

LUA_BINDINGS(bindSession);

static void bindSystemHeuristics(sol::table& m) {
    auto cls = m.new_usertype<SystemHeuristics>("SystemHeuristics");
    cls["galaxy"] = sol::readonly_property(&SystemHeuristics::getGalaxy);
    cls["system"] = sol::readonly_property(&SystemHeuristics::getSystem);
    cls["faction"] = sol::readonly_property(
        [](SystemHeuristics& self) { return self.getFaction() ? sol::optional(self.getFaction()) : sol::nullopt; });
    cls["systems"] = sol::readonly_property(&SystemHeuristics::getSystems);
    cls["sectors"] = sol::readonly_property(&SystemHeuristics::getSectors);
    cls["systems"] = sol::readonly_property(&SystemHeuristics::getSystems);
    cls["find_empty_pos"] = &SystemHeuristics::findEmptyPosition;
}

LUA_BINDINGS(bindSystemHeuristics);

static void bindGenerator(sol::table& m) {
    auto cls = m.new_usertype<Generator>("Generator");
    cls["add_on_start"] = [](Generator& self, sol::function fn) {
        self.addOnStart([fn](uint64_t seed) { LUA_CALL_FN(fn, seed) });
    };
    cls["add_on_galaxy_created"] = [](Generator& self, sol::function fn) {
        self.addOnGalaxyCreated([fn](const GalaxyData& galaxy) { LUA_CALL_FN(fn, sol::as_table(galaxy)) });
    };
    cls["add_on_region_created"] = [](Generator& self, sol::function fn) {
        self.addOnRegionCreated([fn](const GalaxyData& galaxy, const RegionData& region) {
            LUA_CALL_FN(fn, sol::as_table(galaxy), sol::as_table(region))
        });
    };
    cls["add_on_system_created"] = [](Generator& self, sol::function fn) {
        self.addOnSystemCreated(
            [fn](const GalaxyData& galaxy, const SystemData& system, const std::optional<FactionData>& faction) {
                LUA_CALL_FN(fn, sol::as_table(galaxy), sol::as_table(system), sol::optional(faction))
            });
    };
    cls["add_on_sector_created"] = [](Generator& self, sol::function fn) {
        self.addOnSectorCreated([fn](const GalaxyData& galaxy,
                                     const SystemData& system,
                                     const std::optional<FactionData>& faction,
                                     const SectorData& sector) {
            LUA_CALL_FN(fn, sol::as_table(galaxy), sol::as_table(system), sol::optional(faction), sol::as_table(sector))
        });
    };
    cls["add_on_end"] = [](Generator& self, sol::function fn) {
        self.addOnEnd([fn](const GalaxyData& galaxy) { LUA_CALL_FN(fn, sol::as_table(galaxy)) });
    };
    cls["add_on_skipped"] = [](Generator& self, sol::function fn) {
        self.addOnSkipped([fn](const GalaxyData& galaxy) { LUA_CALL_FN(fn, sol::as_table(galaxy)) });
    };
    cls["get_random_name"] = &Generator::getRandomName;
    cls["add_sector_type"] = [](Generator& self, const std::string& name, sol::function fn0, sol::function fn1) {
        self.addSectorType(
            name,
            [fn0](SystemHeuristics& h) {
                LUA_CALL_FN(fn0, h)
                return result.get<float>();
            },
            [fn1](SystemHeuristics& h, uint64_t seed) {
                LUA_CALL_FN(fn1, h, seed)
                return result.get<SectorData>();
            });
    };
}

LUA_BINDINGS(bindGenerator);
