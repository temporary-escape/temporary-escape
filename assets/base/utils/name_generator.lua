local engine = require("engine")

-- Module definition
local name_generator = {}

local random_names_data = {
    "Adara", "Adena", "Adrianne", "Alarice", "Alvita", "Amara", "Ambika", "Antonia", "Araceli",
    "Balandria", "Basha", "Beryl", "Bryn", "Callia", "Caryssa", "Cassandra", "Casondrah", "Chatha",
    "Ciara", "Cynara", "Cytheria", "Dabria", "Darcei", "Deandra", "Deirdre", "Delores", "Desdomna",
    "Devi", "Dominique", "Drucilla", "Duvessa", "Ebony", "Fantine", "Fuscienne", "Gabi", "Gallia",
    "Hanna", "Hedda", "Jerica", "Jetta", "Joby", "Kacila", "Kagami", "Kala", "Kallie",
    "Keelia", "Kerry", "Kerry-Ann", "Kimberly", "Killian", "Kory", "Lilith", "Lucretia", "Lysha",
    "Mercedes", "Mia", "Maura", "Perdita", "Quella", "Riona", "Safiya", "Salina", "Severin",
    "Sidonia", "Sirena", "Solita", "Tempest", "Thea", "Treva", "Trista", "Vala", "Winta",
}

local region_suffixes = {
    "Region",
    "District",
    "Province",
    "Place",
    "Territory",
    "Zone",
    "Division",
    "Domain",
    "Expanse",
    "Realm",
    "Sphere",
    "Vicinity",
    "Enclave",
    "Area",
}

local system_name_generator = engine.NameGenerator.new(random_names_data)

function name_generator.random_system_name(rng)
    return system_name_generator:get(rng)
end

function name_generator.random_region_name(rng)
    local pick = rng:rand_int(1, #region_suffixes)
    local suffix = region_suffixes[pick]
    local name = system_name_generator:get(rng)
    return string.format("%s %s", name, suffix)
end

return name_generator
