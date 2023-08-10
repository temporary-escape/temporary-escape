local engine = require("engine")
local event_bus = engine.get_event_bus()

local logger = engine.create_logger("base/events/event_player.lua")

local module = {}

function module.player_logged_in (event)
    logger:info(string.format("Player has logged in: %s", event.player_id))
end

event_bus:add_handler("player_logged_in", module.player_logged_in)

return module
