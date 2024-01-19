local engine = require("engine")

local EntitySectorAsteroidField = {}

function EntitySectorAsteroidField.new (entity, data)
    local inst = {}
    setmetatable(inst, { __index = EntitySectorAsteroidField })
    inst.entity = entity
    inst:populate()
    return inst
end

function EntitySectorAsteroidField.populate (self)
    local sector = engine.get_sector_data()
    local rng = engine.MT19937.new(sector.seed)

    local scene = engine.get_scene()
    local data = {
        seed = rng:rand_seed(),
    }
    scene:create_entity_template("asteroid_cluster", data)

    for i = 0, 5 do
        local npc = scene:create_entity_template("scout_ship", data)
        print(string.format("%s", npc))
        local transform = npc:get_component_transform()
        transform:translate(engine.Vector3.new(200.0 * i, 20.0, 0.0))
    end
end

return EntitySectorAsteroidField
