local engine = require("engine")
local assets_manager = engine.get_assets_manager()
local server = engine.get_server()

local sector_type = engine.SectorType.new()
sector_type.map_icon = assets_manager:find_image("icon_asteroid_field")
sector_type.weight = 1.0
sector_type.min_count = 1
sector_type.max_count = 4
sector_type.entities = {
    engine.Spawner.new("asteroid_cluster", 1.0, 1),
    engine.Spawner.new("scout_ship", 1.0, 1)
}

server:add_sector_type("asteroid_field", sector_type)

--[[local entity = scene:create_entity()
--
--        local transform = entity:add_component_transform()
--        transform:move(engine.Vector3.new(0.0, -100.0, 0.0))
--
--        local component_model = entity:add_component_model(asteroids[1])
--        local component_rigid_body = entity:add_component_rigid_body()
--        component_rigid_body:set_from_model(component_model.model, 5.0)
--
--        component_rigid_body.linear_velocity = engine.Vector3.new(0.0, 10.0, 0.0)
--
--        entity:add_component_icon(icon)
--        entity:add_component_label("Dynamic")]]--
