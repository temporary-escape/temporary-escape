local engine = require("engine")
local assets_manager = engine.get_assets_manager()

local asteroid_models = {
    assets_manager:find_model("model_asteroid_01_a"),
    assets_manager:find_model("model_asteroid_01_b"),
    assets_manager:find_model("model_asteroid_01_c"),
    assets_manager:find_model("model_asteroid_01_d"),
    assets_manager:find_model("model_asteroid_01_e"),
    assets_manager:find_model("model_asteroid_01_f"),
    assets_manager:find_model("model_asteroid_01_g"),
    assets_manager:find_model("model_asteroid_01_h"),
}

local image_icon = assets_manager:find_image("icon_target")

local EntityAsteroid = {}

function EntityAsteroid.new (entity, data)
    local inst = {}
    setmetatable(inst, { __index = EntityAsteroid })
    inst.entity = entity

    local transform = entity:add_component_transform()
    transform.static = true

    local model_index = (data.seed % #asteroid_models) + 1

    local component_model = entity:add_component_model(asteroid_models[model_index])
    local component_rigid_body = entity:add_component_rigid_body()
    component_rigid_body:set_from_model(component_model.model, data.size)
    component_rigid_body.mass = data.mass

    local icon = entity:add_component_icon(image_icon)
    icon.environment = true
    entity:add_component_label("Asteroid (Rock)")

    return inst
end

return EntityAsteroid
