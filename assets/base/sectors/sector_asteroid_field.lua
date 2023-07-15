local engine = require("engine")
local SectorTemplate = require("base.sectors.sector_template")
local AsteroidCluster = require("base.utils.asteroid_cluster")

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

        local seed = rng:rand_seed()

        local origin = engine.Vector3.new(0.0, 0.0, 0.0)
        local cluster = AsteroidCluster.new(seed, origin, 1500.0, 2.5, 15.0)
        for i = 0, 1000 do
            if cluster:next(scene) then
                local entity = scene:create_entity()

                local transform = entity:add_component_transform()
                transform:move(cluster.pos)
                transform:rotate(cluster.orientation)

                local component_model = entity:add_component_model(asteroids[1])
                local component_rigid_body = entity:add_component_rigid_body()
                component_rigid_body:set_from_model(component_model.model, cluster.size)

                entity:add_component_icon(icon)
                entity:add_component_label("Asteroid (Rock)")

                component_rigid_body.mass = 0.0
                transform.static = true
            end
        end

        --[[
        for x = 0, 44 do
            for y = 0, 44 do
                local entity = scene:create_entity()

                local transform = entity:add_component_transform()
                transform:move(engine.Vector3.new(x * 8.0, 0.0, y * 8.0))

                local component_model = entity:add_component_model(asteroids[1])
                local component_rigid_body = entity:add_component_rigid_body()
                component_rigid_body:set_from_model(asteroids[1], 2.0)

                local component_icon = entity:add_component_icon(
                        icon,
                        engine.Vector2.new(32.0, 32.0),
                        engine.Color4.new(0.7, 0.7, 0.7, 0.5))

                if x == 0 and y == 0 then
                    component_rigid_body.linear_velocity = engine.Vector3.new(5.0, 0.1, 0.0)
                else
                    component_rigid_body.mass = 0.0
                    transform.static = true
                end
            end
        end
        --]]
    end

    return inst
end

local template = SectorAsteroidField.new()

return template
