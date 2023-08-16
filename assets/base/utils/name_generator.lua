local engine = require("engine")

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

local name_generator = engine.NameGenerator.new(random_names_data)

function random_name(rng)
    return name_generator:get(rng)
end

return random_name
