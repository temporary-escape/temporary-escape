local engine = require("engine")
local db = engine.get_database()

db.factions:update("quelis_republic", function(faction)
    if faction == nil then
        faction = engine.FactionData.new()
    end

    faction.id = "quelis_republic"
    faction.name = "The Quelis Republic"
    faction.color = 0.8

    return faction
end)
