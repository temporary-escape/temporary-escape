local engine = require("engine")
local assets_manager = engine.get_assets_manager()

local EntityScoutShip = {}

local image_icon = assets_manager:find_image("icon_target")

function EntityScoutShip.new (entity, data)
    local inst = {}
    setmetatable(inst, { __index = EntityPlayer })
    inst.entity = entity

    local transform = entity:add_component_transform()
    transform:translate(engine.Vector3.new(250.0, 0.0, -150.0))

    local grid = entity:add_component_grid()
    grid:set_from(assets_manager:find_ship_template("player_starter_ship"))

    entity:add_component_icon(image_icon)
    entity:add_component_label("NPC Ship")

    return inst
end

return EntityScoutShip