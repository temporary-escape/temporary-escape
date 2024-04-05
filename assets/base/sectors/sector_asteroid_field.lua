local engine = require("engine")
local assets_manager = engine.get_assets_manager()
local generator = engine.get_generator()

local map_icon = assets_manager:find_image("icon_asteroid_field")

local module = {}

function module.heuristics_callback (heuristics)
    if #heuristics.sectors > 5 then
        return 0.0
    end
    return 1.0
end

function module.on_create_sector (heuristics, seed)
    local rng = engine.MT19937.new(seed)

    local sector = engine.SectorData.new()
    sector.id = engine.uuid()
    sector.name = string.format("%s - Asteroids", heuristics.system.name)
    sector.pos = heuristics:find_empty_pos(rng)
    sector.galaxy_id = heuristics.galaxy.id
    sector.system_id = heuristics.system.id
    sector.seed = rng:rand_seed()
    sector.icon = map_icon
    sector.entity = "entity_sector_asteroid_field"
    return sector
end

generator:add_sector_type("asteroid_field", module.heuristics_callback, module.on_create_sector)

return module
