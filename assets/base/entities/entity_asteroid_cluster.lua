local engine = require("engine")
local AsteroidCluster = require("base.utils.asteroid_cluster")

local EntityAsteroidCluster = {}

function EntityAsteroidCluster.new (entity, data)
    local inst = {}
    setmetatable(inst, { __index = EntityAsteroidCluster })
    inst.entity = entity
    inst.seed = data.seed
    inst:populate()
    return inst
end

function EntityAsteroidCluster.populate (self)
    local rng = engine.MT19937.new(self.seed)
    local scene = engine.get_scene()
    local origin = engine.Vector3.new(0.0, 0.0, 0.0)
    local cluster = AsteroidCluster.new(rng:rand_seed(), origin, 10000.0)

    for _ = 1, 1000 do
        local size = rng:rand_real(10.0, 75.0)

        local data = {
            size = size,
            seed = rng:rand_seed(),
            mass = 0.0, -- Static
        }

        local entity = scene:create_entity_template("asteroid", data)
        local rigid_body = entity:get_component_rigid_body()
        local model = entity:get_component_model()
        local radius = model.model.radius * size

        if cluster:next(radius) then
            rigid_body:reset_transform(cluster.pos, cluster.orientation)
        end
    end
end

return EntityAsteroidCluster
