local engine = require("engine")
local event_bus = engine.get_event_bus()

local logger = engine.create_logger("base/events/event_player.lua")

local module = {}

function module.server_started (event)
    logger:info("Server has started!")
end

event_bus:add_handler("server_started", module.server_started)

return module

