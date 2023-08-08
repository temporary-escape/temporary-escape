local engine = require("engine")

-- Factions
require("base.factions.faction_gallin")
require("base.factions.faction_jetton")
require("base.factions.faction_kaler")
require("base.factions.faction_quelis")
require("base.factions.faction_syndicate")
require("base.factions.faction_terran")
require("base.factions.faction_valas")

-- Event handlers
require("base.events.event_server")
require("base.events.event_player")

-- Sector Templates
--sector_asteroid_field = require("base.sectors.sector_asteroid_field")
--sector_planet_orbits = require("base.sectors.sector_planet_orbits")

--local server = engine.get_server()

-- Prepare the generator that will populate the universe
--local Generator = require("base.generator")
--local generator = Generator.new()

-- Add sector templates to the generator
--generator:add_sector_template(sector_asteroid_field)
--generator:add_sector_template(sector_planet_orbits)

-- Let the server know which generator we want to use
--server:set_generator(function(seed)
--    generator:generate_with_seed(seed)
--end)
