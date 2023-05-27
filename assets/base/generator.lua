local engine = require("engine")
local name_generator = require("base.utils.name_generator")

local logger = engine.create_logger("base/generator.lua")
local db = engine.get_database()
local assets_manager = engine.get_assets_manager()

local function is_in_array(list, item)
    for _, element in pairs(list) do
        if element == item then
            return true
        end
    end
    return false
end

local function pick_array(list, rng)
    if #list == 0 then
        error("Unable to pick randomly from an empty array")
    end
    local pick = rng:rand_int(1, #list)
    return list[pick]
end

local Generator = {
    max_galaxy_width = 250.0,
    max_galaxy_regions = 20,
    max_galaxy_systems = 2000,
    min_galaxy_region_distance = 35,
    min_galaxy_faction_distance = 25,
    max_connection_dist = 40,
    min_system_planets = 3,
    max_system_planets = 6,
    min_planet_moons = 0,
    max_planet_moons = 2,
    min_planet_distance = 8.0,
    max_planet_distance = 12.0,
    min_moon_distance = 0.8,
    max_moon_distance = 2.5,

    sector_templates = {}
}

function Generator.new ()
    o = o or {}
    local meta = {
        __index = Generator
    }
    return setmetatable(o, meta)
end

-- Adds a sector template to be used by this generator
function Generator:add_sector_template (template)
    table.insert(self.sector_templates, template)
end

-- Creates a new galaxy if does not already exists
function Generator:create_main_galaxy (galaxy_id, seed)
    -- No galaxy exists, create one
    local galaxy = engine.GalaxyData.new()
    galaxy.id = galaxy_id
    galaxy.name = "Main Galaxy"
    galaxy.pos = engine.Vector2:new(0.0, 0.0)
    galaxy.seed = seed

    db.galaxies:put(galaxy.id, galaxy)
    galaxy_id = galaxy.id

    -- Make sure to set the metadata that the galaxy exists
    db.metadata:put("main_galaxy_id", galaxy_id)

    logger:info(string.format("Galaxy was generated with id: %s", galaxy_id))
end

-- Populates the galaxy with regions
function Generator:create_galaxy_regions (galaxy_id)
    local galaxy = db.galaxies:find(galaxy_id)
    if galaxy == nil then
        error(string.format("No such galaxy: '%s'", galaxy_id))
    end

    -- Random generator specific to generating regions
    local rng = engine.MT19937.new(galaxy.seed + 10)

    -- Create random positions on a circle with some radius
    -- This makes sure that regions are not too close to each other
    local positions = engine.random_circle_positions(
            rng,
            self.max_galaxy_width * 0.8,
            self.max_galaxy_regions,
            self.min_galaxy_region_distance
    )

    -- Create a region for each position
    for _, pos in pairs(positions) do
        local region = engine.RegionData.new()
        region.id = engine.uuid()
        region.pos = pos
        region.name = name_generator.random_region_name(rng)

        logger:info(string.format("New region pos: %s name: '%s'", pos, region.name))

        db.regions:put(string.format("%s/%s", galaxy_id, region.id), region)
    end

    logger:info(string.format("Created %d regions in galaxy '%s'", #positions, galaxy_id))
end

-- Flood fills the systems with regions
function Generator:fill_galaxy_regions (galaxy_id, positions, connections, systems)
    local regions = db.regions:seek_all(galaxy_id, 0)

    local flood_fill = engine.FloodFill.new()

    for i, region in pairs(regions) do
        flood_fill:add_start_point(region.pos, i)
    end

    for i, pos in pairs(positions) do
        local temp = {}
        for _, conn in pairs(connections[i]) do
            table.insert(temp, conn - 1)
        end
        flood_fill:add_position(pos, temp)
    end

    flood_fill:calculate()

    -- Go through the generated flood map and associate systems
    for i = 1, flood_fill:size() do
        local res = flood_fill:get(i - 1)
        systems[res.index + 1].region_id = regions[res.point].id
    end

    logger:info(string.format("Associated regions with %d systems", #systems))
end

-- Find the nearest system for a given position
local function find_nearest_system (systems, pos)
    local chosen = nil
    for _, system in pairs(systems) do
        if chosen == nil then
            chosen = system
        else
            local a = system.pos:distance(pos)
            local b = chosen.pos:distance(pos)
            if a < b then
                chosen = system
            end
        end
    end

    return chosen
end

-- Find home system for each faction
function Generator:find_faction_homes (galaxy_id, systems)
    local galaxy = db.galaxies:find(galaxy_id)
    if galaxy == nil then
        error(string.format("No such galaxy: '%s'", galaxy_id))
    end

    -- Random generator specific to generating regions
    local rng = engine.MT19937.new(galaxy.seed + 10)

    -- Get all factions
    local factions = db.factions:seek_all("", 0)

    -- Generate random positions across the galaxy
    local rand_positions = engine.random_circle_positions(
            rng,
            self.max_galaxy_width * 0.8,
            #factions,
            self.min_galaxy_faction_distance
    )

    -- Make sure we have all of the points
    if #rand_positions ~= #factions then
        error("Failed to generate faction positions")
    end

    for i, pos in pairs(rand_positions) do
        local system = find_nearest_system(systems, pos)
        db.factions:update(factions[i].id, function(faction)
            faction.home_galaxy_id = galaxy_id
            faction.home_system_id = system.id
            return faction
        end)
    end
end

-- Flood fills the systems with factions
function Generator:fill_galaxy_factions (galaxy_id, positions, connections, indexes, systems)
    local factions = db.factions:seek_all("", 0)

    local flood_fill = engine.FloodFill.new()

    for i, faction in pairs(factions) do
        local system = systems[indexes[faction.home_system_id]]
        flood_fill:add_start_point(system.pos, i)
    end

    for i, pos in pairs(positions) do
        local temp = {}
        for _, conn in pairs(connections[i]) do
            table.insert(temp, conn - 1)
        end
        flood_fill:add_position(pos, temp)
    end

    flood_fill:calculate()

    local counts = {}

    -- Go through the generated flood map and associate systems
    local total = 0
    for i = 1, flood_fill:size() do
        local res = flood_fill:get(i - 1)
        local faction = factions[res.point]

        if counts[faction.id] == nil then
            counts[faction.id] = 0
        end

        if counts[faction.id] < 120 then
            systems[res.index + 1].faction_id = factions[res.point].id

            total = total + 1
            counts[faction.id] = counts[faction.id] + 1
        end
    end

    logger:info(string.format("Associated factions with %d systems", total))
end

-- Populates the system with planets
function Generator:create_system_planets (galaxy, system)
    local planet_types = assets_manager:find_all_planet_types()

    local rng = engine.MT19937.new(galaxy.seed + 10)

    local orbit_distance = 0

    -- Create planets for this system
    local total_planets = rng:rand_int(self.min_system_planets, self.max_system_planets)
    for p = 1, total_planets do
        local planet = engine.PlanetData.new()
        planet.id = engine.uuid()
        planet.name = string.format("%s %d", system.name, p)
        planet.galaxy_id = galaxy.id
        planet.system_id = system.id
        planet.parent = nil
        planet.type = pick_array(planet_types, rng)

        local moon_orbit_distance = 0

        local moons = {}

        -- Moons for the planet
        local total_moons = rng:rand_int(self.min_planet_moons, self.max_planet_moons)
        for m = 1, total_moons do
            local moon = engine.PlanetData.new()
            moon.id = engine.uuid()
            moon.name = string.format("%s %d - %d", system.name, p, m)
            moon.galaxy_id = galaxy.id
            moon.system_id = system.id
            moon.parent = planet.id
            moon.type = pick_array(planet_types, rng)

            local rand_distance = rng:rand_real(self.min_moon_distance, self.max_moon_distance)
            moon_orbit_distance = moon_orbit_distance + rand_distance
            moon.pos = engine.Vector2.new(moon_orbit_distance, 0.0)

            table.insert(moons, moon)
        end

        local rand_distance = rng:rand_real(self.min_planet_distance, self.max_planet_distance)
        orbit_distance = orbit_distance + rand_distance + moon_orbit_distance

        planet.pos = engine.Vector2.new(orbit_distance, 0.0)
        planet.pos = planet.pos:rotate(rng:rand_real(0.0, 6.28318530718))

        for _, moon in pairs(moons) do
            moon.pos = moon.pos:rotate(rng:rand_real(0.0, 6.28318530718)) + planet.pos

            db.planets:put(string.format("%s/%s/%s", galaxy.id, system.id, moon.id), moon)
        end

        orbit_distance = orbit_distance + moon_orbit_distance
        db.planets:put(string.format("%s/%s/%s", galaxy.id, system.id, planet.id), planet)
    end
end

--- Populates the system with sectors
function Generator:create_system_sectors (rng, galaxy, system)
    local planets = db.planets:seek_all(string.format("%s/%s/", galaxy.id, system.id), 0)

    for _, template in pairs(self.sector_templates) do
        template:generate(rng, galaxy, system, planets)
    end
end

-- Populates the galaxy systems with sectors
function Generator:create_galaxy_sectors (galaxy_id)
    if #self.sector_templates == 0 then
        error("Can not generate sectors, no system templates present")
    end

    local galaxy = db.galaxies:find(galaxy_id)
    if galaxy == nil then
        error(string.format("No such galaxy: '%s'", galaxy_id))
    end

    local rng = engine.MT19937.new(galaxy.seed + 30)

    local it = db.systems:seek(string.format("%s/", galaxy.id), nil)
    while it:next() do
        self:create_system_sectors(rng, galaxy, it.value)
    end
end

-- Populates the galaxy with systems
function Generator:create_galaxy_systems (galaxy_id)
    local galaxy = db.galaxies:find(galaxy_id)
    if galaxy == nil then
        error(string.format("No such galaxy: '%s'", galaxy_id))
    end

    -- Random generator specific to generating regions
    local rng = engine.MT19937.new(galaxy.seed + 20)

    -- Create randomly placed systems
    local systems = {}
    local names = {}
    local distribution = engine.GalaxyDistribution.new(self.max_galaxy_width, 2.5, 1.2)
    for i = 1, self.max_galaxy_systems do
        local pos = distribution:get(rng)

        -- Unable to fit more systems into the galaxy
        if pos == nil then
            logger:warn(string.format("Unable to get next position after %d iteration", i))
            break
        end

        -- Find a unique name for the system
        local tries = 10
        local name = nil
        while tries > 0 do
            name = name_generator.random_system_name(rng)
            if names[name] == nil then
                names[name] = true
                break
            end
            name = nil
            tries = tries - 1
        end

        -- Do not continue if we can't find a unique name
        if name == nil then
            logger:warn(string.format("Unable to find unique name after %d iteration", i))
            break
        end

        local system = engine.SystemData.new()
        system.id = engine.uuid()
        system.pos = pos
        system.galaxy_id = galaxy_id;
        system.name = name_generator.random_system_name(rng)
        system.seed = rng:rand_seed()

        table.insert(systems, system)
    end

    logger:info(string.format("Galaxy distribution created %d systems", #systems))

    -- Connect systems together
    local positions = {}
    for i, system in pairs(systems) do
        positions[i] = system.pos
    end

    -- Minimum spanning tree ensures that all systems are connected to each other
    local min_tree_map = engine.MinimumSpanningTree.new()
    for _, system in pairs(systems) do
        min_tree_map:add_position(system.pos)
    end
    min_tree_map:calculate()

    -- Delaunay triangulation creates triangle-like connections
    local triangulation_map = engine.DelaunayTriangulation.new()
    for _, system in pairs(systems) do
        triangulation_map:add_position(system.pos)
    end
    triangulation_map:calculate()

    -- Go through all systems
    for i, system in pairs(systems) do
        -- Get the connections for this system
        if triangulation_map:has_connections(i - 1) then
            local connections = triangulation_map:get_connections(i - 1)
            if #connections == 0 then
                error(string.format("Delaunary triangulation failed system: '{}' has no connection", system.id))
            end

            -- For each connection coming out of this system
            for _, conn in pairs(connections) do
                local other = systems[conn + 1]
                local should_discard = false

                -- Discard connection if it is too far from each other
                if system.pos:distance(other.pos) > self.max_connection_dist then
                    should_discard = true
                end

                -- Discard connection randomly
                if rng:rand_real(0.0, 1.0) < 0.6 then
                    should_discard = true
                end

                -- Do not discard if this is part of minimal spanning tree
                if should_discard == true then
                    if min_tree_map:has_connections(i - 1) then
                        local min_conns = min_tree_map:get_connections(i - 1)
                        if is_in_array(min_conns, conn) then
                            should_discard = false
                        end
                    end

                    if min_tree_map:has_connections(conn) then
                        local min_conns = min_tree_map:get_connections(conn)
                        if is_in_array(min_conns, i - 1) then
                            should_discard = false
                        end
                    end
                end

                if should_discard == false then
                    -- Insert if not self and not already included
                    local exists = is_in_array(system.connections, other.id)
                    if exists == false and system.id ~= other.id then
                        table.insert(system.connections, other.id)
                    end

                    exists = is_in_array(other.connections, system.id)
                    if exists == false and system.id ~= other.id then
                        table.insert(other.connections, system.id)
                    end
                end
            end
        end
    end

    -- Validate all systems have at least one connection
    for i, system in pairs(systems) do
        if #system.connections == 0 then
            error("Generating connections failed, not all systems have connections")
        end
    end

    logger:info(string.format("Generated connections for %d systems", #systems))

    -- Index map converts a system id into an index
    local indexes = {}
    for i, system in pairs(systems) do
        indexes[system.id] = i
    end

    -- Connection map is a map of integer -> list of integers
    local connections = {}
    for i, system in pairs(systems) do
        connections[i] = {}
        for _, other_id in pairs(system.connections) do
            table.insert(connections[i], indexes[other_id])
        end
    end

    self:fill_galaxy_regions(galaxy_id, positions, connections, systems)
    self:find_faction_homes(galaxy_id, systems)
    self:fill_galaxy_factions(galaxy_id, positions, connections, indexes, systems)

    for i, system in pairs(systems) do
        self:create_system_planets(galaxy, system)

        local key = string.format("%s/%s", galaxy_id, system.id);
        db.systems:put(key, system)
    end

    logger:info(string.format("Created %d systems", #systems))
end

-- The entrypoint for our generator function
function Generator:generate_with_seed (seed)
    logger:info(string.format("Generating world with seed: %d", seed))

    local galaxy_id = db.metadata:find("main_galaxy_id")
    if galaxy_id == nil then
        logger:info("Galaxy is not generated, creating one...")
        galaxy_id = engine.uuid()

        self:create_main_galaxy(galaxy_id, seed)
        self:create_galaxy_regions(galaxy_id)
        self:create_galaxy_systems(galaxy_id)
        self:create_galaxy_sectors(galaxy_id)

        logger:info(string.format("Galaxy was generated id: %s", galaxy_id))
    else
        logger:info(string.format("Galaxy is already generated id: %s", galaxy_id))
    end
end

return Generator
