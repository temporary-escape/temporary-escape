local engine = require("engine")
local event_bus = engine.get_event_bus()
local server = engine.get_server()
local db = engine.get_database()

local logger = engine.create_logger("base/events/event_player.lua")

local module = {}

function module.init_player_location (player_id)
    -- Check if the player has a location
    -- If not, create one
    local location = db.player_locations:find(player_id)
    if location == nil then
        logger:info("Player has no starting location, creating one...")

        -- Get the starting locations
        local starting_locations = db.starting_locations:seek_all("", 1)
        if #starting_locations == 0 then
            error("There are no starting locations")
        end

        local starting_location = starting_locations[1]

        -- Create the location data
        location = engine.PlayerLocationData.new()
        location.galaxy_id = starting_location.galaxy_id
        location.system_id = starting_location.system_id
        location.sector_id = starting_location.sector_id

        db.player_locations:put(player_id, location)
    else
        logger:info("Player has starting location")
    end

    return location
end

function module.player_logged_in (event)
    logger:info(string.format("Player has logged in: %s", event.player_id))

    local location = module.init_player_location(event.player_id)

    server:start_sector(location.galaxy_id, location.system_id, location.sector_id)
    server:move_player_to_sector(event.player_id, location.sector_id)
end

event_bus:add_handler("player_logged_in", module.player_logged_in)

return module
