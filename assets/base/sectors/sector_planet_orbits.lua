local engine = require("engine")
local SectorTemplate = require("base.sectors.sector_template")

local logger = engine.create_logger("base/sectors/sector_planet_orbits.lua")

local db = engine.get_database()
local assets_manager = engine.get_assets_manager()

local SectorPlanetOrbits = {}

function SectorPlanetOrbits.new()
    local inst = SectorTemplate.new()

    function inst:generate(rng, galaxy, system, planets, faction)
        for _, planet in pairs(planets) do
            local sector = engine.SectorData.new()
            sector.id = engine.uuid()
            sector.name = string.format("%s", planet.name)

            -- Create a position along the orbit of the planet
            local offset = engine.Vector2.new(planet.radius + 0.2, 0.0)
            offset = offset:rotate(engine.radians(rng:rand_real(0.0, 360.0)))

            sector.pos = planet.pos + offset

            sector.galaxy_id = galaxy.id
            sector.system_id = system.id
            sector.seed = rng:rand_seed()
            sector.icon = assets_manager:find_image("icon_ringed_planet")
            sector.template = "base.sectors.sector_planet_orbits"

            key = string.format("%s/%s/%s", galaxy.id, system.id, sector.id);
            db.sectors:put(key, sector)
        end
    end

    function inst:populate(rng, galaxy, system, sector, scene)
        local key = string.format("%s/%s/%s", galaxy.id, system.id, sector.id);
        logger:info(string.format("Populating sector: %s", key))
    end

    return inst
end

local template = SectorPlanetOrbits.new()

return template
