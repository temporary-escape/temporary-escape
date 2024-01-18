local engine = require("engine")

local scene = engine.get_scene()

-- Sector entrypoints
scene:add_entity_template("entity_sector_asteroid_field", require("base.entities.entity_sector_asteroid_field"))

-- All entities
scene:add_entity_template("asteroid", require("base.entities.entity_asteroid"))
scene:add_entity_template("asteroid_cluster", require("base.entities.entity_asteroid_cluster"))
scene:add_entity_template("player", require("base.entities.entity_player"))
scene:add_entity_template("scout_ship", require("base.entities.entity_scout_ship"))
