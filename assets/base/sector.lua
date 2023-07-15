local engine = require("engine")

entities = {}

entities["asteroid"] = require("base.entities.entity_asteroid")

function spawn(name, ...)
    if entities[name] == nil then
        error(string.format("No such entity: %s", name))
    end

    local scene = engine.get_scene()

    local entity = scene:create_entity()
    local klass = entities[name]

    local instance = klass.new(entity, ...)
    instance:create(...)
    entity:add_component_script(instance)
    return entity
end
