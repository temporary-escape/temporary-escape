local SectorTemplate = {}

function SectorTemplate.new ()
    local inst = {}
    setmetatable(inst, { __index = SectorTemplate })
    return inst
end

function SectorTemplate:generate(rng, galaxy, system, planets, faction)
    error("Function generate() not implemented")
end

function SectorTemplate:populate(rng, galaxy, system, sector, scene)
    error("Function populate() not implemented")
end

return SectorTemplate
