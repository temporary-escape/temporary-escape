local EntityAsteroid = {}

function EntityAsteroid.new (entity)
    local inst = {}
    setmetatable(inst, { __index = EntityAsteroid })
    inst.entity = entity
    return inst
end

function EntityAsteroid:create (data)
    local transform = self.entity:add_component_transform()
    transform.static = true

    local component_model = self.entity:add_component_model(data.model)
    --local component_rigid_body = self.entity:add_component_rigid_body()
    --component_rigid_body:set_from_model(component_model.model, data.size)
    --component_rigid_body.mass = 0.0

    self.entity:add_component_icon(data.icon)
    self.entity:add_component_label("Asteroid (Rock)")
end

return EntityAsteroid
