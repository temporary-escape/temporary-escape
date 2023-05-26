local engine = require("engine")
local event_bus = engine.get_event_bus()

local logger = engine.create_logger("base/events/event_server.lua")

event_bus:add_handler("server_started", function(data)
    logger:info("Server has started!")
end)
