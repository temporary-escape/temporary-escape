local engine = require("engine")
local db = engine.get_database()

db.factions:update("terran_empire", function(faction)
    if faction == nil then
        faction = engine.FactionData.new()
    end

    faction.id = "terran_empire"
    faction.name = "The Terran Empire"
    faction.color = 0.15

    return faction
end)
