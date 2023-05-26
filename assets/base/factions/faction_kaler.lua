local engine = require("engine")
local db = engine.get_database()

db.factions:update("kaler_state", function(faction)
    if faction == nil then
        faction = engine.FactionData.new()
    end

    faction.id = "kaler_state"
    faction.name = "The Kaler State"
    faction.color = 0.25

    return faction
end)
