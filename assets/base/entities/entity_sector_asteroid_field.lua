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

    for x = 0, 9 do
        for y = 0, 2 do
            local npc = scene:create_entity_template("scout_ship", data)
            npc.transform:translate(engine.Vector3.new(100.0 * x, 50.0, 100.0 * y))
        end
    end
end

return EntitySectorAsteroidField
