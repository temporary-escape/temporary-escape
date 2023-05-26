local engine = require("engine")
local db = engine.get_database()

db.factions:update("jetton_kingdom", function(faction)
    if faction == nil then
        faction = engine.FactionData.new()
    end

    faction.id = "jetton_kingdom"
    faction.name = "The Jetton Kingdom"
    faction.color = 0.4

    return faction
end)
