local engine = require("engine")
local db = engine.get_database()

db.factions:update("syndicate", function(faction)
    if faction == nil then
        faction = engine.FactionData.new()
    end

    faction.id = "syndicate"
    faction.name = "The Syndicate"
    faction.color = 0.0

    return faction
end)
