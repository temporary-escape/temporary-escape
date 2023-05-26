local engine = require("engine")
local db = engine.get_database()

db.factions:update("valas_corporation", function(faction)
    if faction == nil then
        faction = engine.FactionData.new()
    end

    faction.id = "valas_corporation"
    faction.name = "The Valas Corporation"
    faction.color = 0.6

    return faction
end)
