local SectorTemplate = {
    icon = nil,
}

function SectorTemplate.new ()
    o = o or {}
    local meta = {
        __index = SectorTemplate
    }
    return setmetatable(o, meta)
end

function SectorTemplate:generate(rng, galaxy, system, planets)
    error("Not implemented")
end

return SectorTemplate
