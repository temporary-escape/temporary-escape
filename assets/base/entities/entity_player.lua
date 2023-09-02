local engine = require("engine")
local assets_manager = engine.get_assets_manager()
local db = engine.get_database()
local scene = engine.get_scene()

local EntityPlayer = {}

local image_icon = assets_manager:find_image("icon_target")

function EntityPlayer.new (entity, data)
    local inst = {}
    setmetatable(inst, { __index = EntityPlayer })
    inst.entity = entity

    local transform = entity:add_component_transform()
    transform:translate(engine.Vector3.new(200.0, -20.0, 0.0))

    local ship_control = entity:add_component_ship_control()

    local grid = entity:add_component_grid()
    grid:set_from(assets_manager:find_ship_template("player_starter_ship"))

    -- Get the player data
    local player = db.players:get(data.player_id)

    entity:add_component_icon(image_icon)
    entity:add_component_label(player.name)

    -- Turret
    local child = scene:create_entity()
    local child_transform = child:add_component_transform()
    child_transform:translate(engine.Vector3.new(0.0, 1.5, 0.0))
    child_transform.parent = transform
    child:add_component_model_skinned(assets_manager:find_model("model_turret_projectile_01"))
    local turret = child:add_component_turret(assets_manager:find_turret("turret_projectile_01"))
    ship_control:add_turret(turret)

    child = scene:create_entity()
    child_transform = child:add_component_transform()
    child_transform:translate(engine.Vector3.new(0.0, 0.5, -6.0))
    child_transform.parent = transform
    child:add_component_model_skinned(assets_manager:find_model("model_turret_projectile_01"))
    turret = child:add_component_turret(assets_manager:find_turret("turret_projectile_01"))
    ship_control:add_turret(turret)

    return inst
end

return EntityPlayer