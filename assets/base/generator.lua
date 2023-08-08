local engine = require("engine")
local name_generator = require("base.utils.name_generator")

local logger = engine.create_logger("base/generator.lua")
local db = engine.get_database()
local assets_manager = engine.get_assets_manager()

--- Populates the system with sectors
function Generator:create_system_sectors (rng, galaxy, system)
    local planets = db.planets:seek_all(string.format("%s/%s/", galaxy.id, system.id), 0)

    local faction = nil
    if system.faction_id ~= nil then
        faction = db.factions:get(system.faction_id)
    end

    for _, template in pairs(self.sector_templates) do
        template:generate(rng, galaxy, system, planets, faction)
    end
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
