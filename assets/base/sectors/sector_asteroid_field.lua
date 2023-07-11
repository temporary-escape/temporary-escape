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

        local icon = assets_manager:find_image("icon_target")

        for x = 0, 15 do
            for y = 0, 15 do
                local entity = scene:create_entity()

                local transform = entity:add_component_transform()
                transform:move(engine.Vector3.new(x * 8.0, 0.0, y * 8.0))
                --transform:scale(engine.Vector3.new(2.0, 2.0, 2.0))

                local component_model = entity:add_component_model(asteroids[1])
                local component_rigid_body = entity:add_component_rigid_body(asteroids[1], 2.0)

                local component_icon = entity:add_component_icon(
                        icon,
                        engine.Vector2.new(48.0, 48.0),
                        engine.Color4.new(1.0, 0.0, 0.0, 1.0))

                if x == 0 and y == 0 then
                    component_rigid_body.linear_velocity = engine.Vector3.new(5.0, 0.1, 0.0)
                else
                    component_rigid_body.mass = 0.0
                    component_model.static = true
                    --transform:scale(engine.Vector3.new(2.0, 2.0, 2.0))
                end
            end
        end
    end

    return inst
end

local template = SectorAsteroidField.new()

return template
