local engine = require("engine")
local db = engine.get_database()

db.factions:update("gallin_federation", function(faction)
    if faction == nil then
        faction = engine.FactionData.new()
    end

    faction.id = "gallin_federation"
    faction.name = "The Gallin Federation"
    faction.color = 0.5

    return faction
end)
