local engine = require("engine")

local scene = engine.get_scene()

scene:add_entity_template("asteroid", require("base.entities.entity_asteroid"))
scene:add_entity_template("asteroid_cluster", require("base.entities.entity_asteroid_cluster"))
scene:add_entity_template("player", require("base.entities.entity_player"))
scene:add_entity_template("scout_ship", require("base.entities.entity_scout_ship"))
