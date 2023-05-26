local engine = require("engine")
local SectorTemplate = require("base.sectors.sector_template")

local db = engine.get_database()

local template = SectorTemplate.new()

function template:generate(rng, galaxy, system, planets)
    local sector = engine.SectorData.new()
    sector.id = engine.uuid()
    sector.name = string.format("%s Sector", system.name)
    sector.pos = engine.Vector2.new(0.0, 0.0)
    sector.galaxy_id = galaxy.id
    sector.system_id = system.id
    sector.seed = rng:rand_seed()
    key = string.format("%s/%s/%s", galaxy.id, system.id, sector.id);
    db.sectors:put(key, sector)
end

return template
