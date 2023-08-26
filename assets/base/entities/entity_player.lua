local engine = require("engine")
local assets_manager = engine.get_assets_manager()
local db = engine.get_database()

local EntityPlayer = {}

local icon = assets_manager:find_image("icon_target")

function EntityPlayer.new (entity, data)
    local inst = {}
    setmetatable(inst, { __index = EntityPlayer })
    inst.entity = entity

    local transform = entity:add_component_transform()
    transform:translate(engine.Vector3.new(200.0, 0.0, 0.0))

    local grid = entity:add_component_grid()
    grid:set_from(assets_manager:find_ship_template("player_starter_ship"))

    -- Get the player data
    local player = db.players:get(data.player_id)

    entity:add_component_icon(icon)
    entity:add_component_label(player.name)

    return inst
end

return EntityPlayer
