local engine = require("engine")
local event_bus = engine.get_event_bus()

local logger = engine.create_logger("base/events/event_player.lua")

local module = {}

function module.sector_player_added (event)
    logger:info(string.format("Player: %s added to sector: %s", event.player_id, event.sector_id))
end

event_bus:add_handler("sector_player_added", module.sector_player_added)

return module
