local engine = require("engine")
local generator = engine.get_generator()
local db = engine.get_database()

local logger = engine.create_logger("base/generator.lua")

local module = {}

function module.on_start(seed)
    logger:info(string.format("Generator started with seed: %d", seed))
end

function module.on_end(galaxy)
    logger:info("Generator ended")
end

function module.on_sector_created(galaxy, system, faction, sector)
    local location = engine.StartingLocationData.new()
    location.galaxy_id = galaxy.id
    location.system_id = system.id
    location.sector_id = sector.id
    db.starting_locations:put(sector.id, location)
end

generator:add_on_start(module.on_start)
generator:add_on_end(module.on_end)
generator:add_on_sector_created(module.on_sector_created)

return module
