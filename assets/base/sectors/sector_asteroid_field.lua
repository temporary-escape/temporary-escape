local engine = require("engine")
local SectorTemplate = require("base.sectors.sector_template")

local logger = engine.create_logger("base/sectors/sector_asteroid_field.lua")

local db = engine.get_database()
local assets_manager = engine.get_assets_manager()

local SectorAsteroidField = {}

function SectorAsteroidField.new()
    local inst = SectorTemplate.new()

    function inst:generate(rng, galaxy, system, planets, faction)
        local sector = engine.SectorData.new()
        sector.id = engine.uuid()
        sector.name = string.format("%s Sector", system.name)
        sector.pos = engine.Vector2.new(0.0, 0.0)
        sector.galaxy_id = galaxy.id
        sector.system_id = system.id
        sector.seed = rng:rand_seed()
        sector.icon = assets_manager:find_image("icon_asteroid_field")
        sector.template = "base.sectors.sector_asteroid_field"

        local key = string.format("%s/%s/%s", galaxy.id, system.id, sector.id);
        db.sectors:put(key, sector)

        -- Mark this sector as a starting location
        local starting_location = engine.StartingLocationData.new()
        starting_location.galaxy_id = galaxy.id
        starting_location.system_id = system.id
        starting_location.sector_id = sector.id
        db.starting_locations:put(engine.uuid(), starting_location)
    end

    function inst:populate(rng, galaxy, system, sector, scene)
        local key = string.format("%s/%s/%s", galaxy.id, system.id, sector.id);
        logger:info(string.format("Populating sector: %s", key))

        local asteroids = {
            assets_manager:find_model("model_asteroid_01_a"),
            assets_manager:find_model("model_asteroid_01_b"),
            assets_manager:find_model("model_asteroid_01_c"),
            assets_manager:find_model("model_asteroid_01_d"),
            assets_manager:find_model("model_asteroid_01_e"),
            assets_manager:find_model("model_asteroid_01_f"),
            assets_manager:find_model("model_asteroid_01_g"),
            assets_manager:find_model("model_asteroid_01_h"),
        }

        for x = 0, 20 do
            local entity = scene:create_entity()

            local transform = entity:add_component_transform()
            transform:move(engine.Vector3.new(x * 3.0, 0.0, 0.0))

            local component_model = entity:add_component_model()
            component_model.model = asteroids[1]

            local component_rigid_body = entity:add_component_rigid_body()
            component_rigid_body:set_from_model(asteroids[1])

            --if x == 0 then
            --    component_rigid_body:set_linear_velocity(engine.Vector3.new(3.0, 0.1, 0.0))
            --end
        end
    end

    return inst
end

local template = SectorAsteroidField.new()

return template
