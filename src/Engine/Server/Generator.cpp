#include "Generator.hpp"
#include "../Math/DelaunayTriangulation.hpp"
#include "../Math/FloodFill.hpp"
#include "../Math/GalaxyDistribution.hpp"
#include "../Math/MinimumSpanningTree.hpp"
#include "../Math/Random.hpp"
#include "../Server/Lua.hpp"
#include "../Utils/NameGenerator.hpp"
#include "../Utils/Random.hpp"
#include <sol/sol.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

const std::vector<std::string> randomNamesData = {
    "Adara",     "Adena",     "Adrianne",  "Alarice",  "Alvita",  "Amara",   "Ambika",    "Antonia",   "Araceli",
    "Balandria", "Basha",     "Beryl",     "Bryn",     "Callia",  "Caryssa", "Cassandra", "Casondrah", "Chatha",
    "Ciara",     "Cynara",    "Cytheria",  "Dabria",   "Darcei",  "Deandra", "Deirdre",   "Delores",   "Desdomna",
    "Devi",      "Dominique", "Drucilla",  "Duvessa",  "Ebony",   "Fantine", "Fuscienne", "Gabi",      "Gallia",
    "Hanna",     "Hedda",     "Jerica",    "Jetta",    "Joby",    "Kacila",  "Kagami",    "Kala",      "Kallie",
    "Keelia",    "Kerry",     "Kerry-Ann", "Kimberly", "Killian", "Kory",    "Lilith",    "Lucretia",  "Lysha",
    "Mercedes",  "Mia",       "Maura",     "Perdita",  "Quella",  "Riona",   "Safiya",    "Salina",    "Severin",
    "Sidonia",   "Sirena",    "Solita",    "Tempest",  "Thea",    "Treva",   "Trista",    "Vala",      "Winta",
};

const std::vector<std::string> regionSuffixes = {
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
};

static const auto randomNameGenerator = NameGenerator{randomNamesData};

static std::string randomRegionName(Rng& rng) {
    const auto pick = randomInt<size_t>(rng, 0, regionSuffixes.size() - 1);
    const auto suffix = regionSuffixes.at(pick);
    const auto name = randomNameGenerator(rng);
    return fmt::format("{} {}", name, suffix);
}

template <typename T> static bool isInArray(const std::vector<T>& a, const T& b) {
    return std::find_if(a.begin(), a.end(), [&](const T& i) { return i == b; }) != a.end();
}

static const RegionData& getNearestRegion(const std::vector<RegionData>& regions, const Vector2& pos) {
    const RegionData* ptr = nullptr;
    for (const auto& region : regions) {
        if (ptr == nullptr) {
            ptr = &region;
        } else {
            const auto a = glm::distance(pos, region.pos);
            const auto b = glm::distance(pos, ptr->pos);

            if (a < b) {
                ptr = &region;
            }
        }
    }

    if (ptr == nullptr) {
        EXCEPTION("Unable to find nearest region");
    }
    return *ptr;
}

static const SystemData& getNearestSystem(const std::vector<SystemData>& systems, const Vector2& pos) {
    const SystemData* ptr = nullptr;
    for (const auto& system : systems) {
        if (ptr == nullptr) {
            ptr = &system;
        } else {
            const auto a = glm::distance(pos, system.pos);
            const auto b = glm::distance(pos, ptr->pos);

            if (a < b) {
                ptr = &system;
            }
        }
    }

    if (ptr == nullptr) {
        EXCEPTION("Unable to find nearest system");
    }
    return *ptr;
}

Generator::Generator(const Options& options, AssetsManager& assetsManager, Database& db) :
    options{options}, assetsManager{assetsManager}, db{db} {
}

void Generator::generate(const uint64_t seed) {
    logger.info("Generating world with seed: {}", seed);

    auto mainGalaxyId = db.find<MetaData>("main_galaxy_id");
    if (!mainGalaxyId) {
        logger.info("Galaxy is not generated, creating one...");

        const auto galaxyId = uuid();

        mainGalaxyId = MetaData{};
        mainGalaxyId->value = galaxyId;

        Rng rng{seed};

        createMainGalaxy(galaxyId, randomSeed(rng));
        db.put<MetaData>("main_galaxy_id", *mainGalaxyId);

        createGalaxyRegions(galaxyId);
        createGalaxySystems(galaxyId);
        createGalaxySectors(galaxyId);
    }
}

void Generator::addSectorType(std::string name, SectorType type) {
    const auto it = sectorTypes.emplace(std::move(name), std::move(type));
    logger.info("Added sector type: {}", it.first->first);
}

void Generator::populate(const SectorData& sector, Scene& scene) {
    std::mt19937_64 rng{sector.seed};

    const auto it = sectorTypes.find(sector.type);
    if (it == sectorTypes.end()) {
        EXCEPTION("No such sector type: {}", sector.type);
    }

    logger.info("Populating sector: {} with type: {}", sector.id, it->first);

    for (const auto& we : it->second.entities) {
        const auto weight = randomReal<float>(rng, 0.0f, 1.0f);
        if (weight < we.weight) {
            auto table = scene.getLua().getState().create_table();
            table["seed"] = randomSeed(rng);
            scene.createEntityFrom(we.entity, table);
        }
    }
}

void Generator::createMainGalaxy(const std::string& galaxyId, const uint64_t seed) {
    auto galaxy = GalaxyData{};
    galaxy.id = galaxyId;
    galaxy.name = "Main Galaxy";
    galaxy.pos = Vector2{0.0f, 0.0f};
    galaxy.seed = seed;

    db.put<GalaxyData>(galaxyId, galaxy);

    logger.info("Galaxy was generated with id: {}", galaxyId);
}

void Generator::createGalaxyRegions(const std::string& galaxyId) {
    const auto galaxy = db.get<GalaxyData>(galaxyId);

    Rng rng{galaxy.seed + 10};

    const auto positions = randomCirclePositions(
        rng, options.galaxyWidthMax * 0.8f, options.galaxyRegionsMax, options.galaxyRegionDistanceMin);

    for (const auto& pos : positions) {
        auto region = RegionData{};
        region.id = uuid();
        region.pos = pos;
        region.name = randomRegionName(rng);

        db.put<RegionData>(fmt::format("{}/{}", galaxyId, region.id), region);
    }

    logger.info("Created {} regions in galaxy {}", positions.size(), galaxyId);
}

void Generator::createGalaxySystems(const std::string& galaxyId) {
    const auto galaxy = db.get<GalaxyData>(galaxyId);

    // Random generator specific to generating systems
    Rng rng{galaxy.seed + 20};

    std::unordered_set<std::string> names;

    std::vector<SystemData> systems;
    systems.reserve(options.galaxySystemsMax);

    // Create randomly placed systems
    auto distribution = GalaxyDistribution{options.galaxyWidthMax, 2.5f, 1.2f};
    for (size_t i = 0; i < options.galaxySystemsMax; i++) {
        const auto pos = distribution(rng);

        // Unable to fit more systems into the galaxy
        if (!pos) {
            logger.warn("Unable to get next system position after {} iterations", i);
            break;
        }

        // Find a unique name for the system
        std::string name;
        auto tries = 10;
        while (tries > 0) {
            name = randomNameGenerator(rng);
            if (names.find(name) == names.end()) {
                names.insert(name);
                break;
            }
            --tries;
        }

        // Do not continue if we can't find a unique name
        if (name.empty()) {
            logger.warn("Unable to find unique name after {} iterations", i);
            break;
        }

        systems.emplace_back();
        auto& systemData = systems.back();
        systemData.id = uuid();
        systemData.pos = *pos;
        systemData.galaxyId = galaxyId;
        systemData.name = name;
        systemData.seed = randomSeed(rng);
    }

    logger.info("Galaxy distribution created {} systems in galaxy {}", systems.size(), galaxyId);

    // Connect systems together, prepare list of positions first
    std::vector<Vector2> positions;
    positions.reserve(systems.size());
    for (const auto& system : systems) {
        positions.emplace_back(system.pos);
    }

    // Minimum spanning tree ensures that all systems are connected to each other
    auto minTreeMap = MinimumSpanningTree{};
    for (const auto& pos : positions) {
        minTreeMap.addPosition(pos);
    }
    minTreeMap.calculate();

    // Delaunay triangulation creates triangle-like connections
    auto triangulationMap = DelaunayTriangulation{};
    for (const auto& pos : positions) {
        triangulationMap.addPosition(pos);
    }
    triangulationMap.calculate();

    // Go through all systems
    for (size_t i = 0; i < systems.size(); i++) {
        auto& system = systems.at(i);

        // Get the connections for this system
        if (triangulationMap.hasConnections(i)) {
            const auto& connections = triangulationMap.getConnections(i);
            if (connections.empty()) {
                EXCEPTION("Delaunary triangulation failed system: '{}' has no connection", system.id);
            }

            // For each connection coming out of this system
            for (const auto& conn : connections) {
                auto& other = systems.at(conn);
                auto shouldDiscard = false;

                // Discard connection if it is too far from each other
                if (glm::distance(system.pos, other.pos) > options.connectionDistMax) {
                    shouldDiscard = true;
                }

                // Discard connection randomly
                if (!shouldDiscard && randomReal(rng, 0.0f, 1.0f) < 0.6f) {
                    shouldDiscard = true;
                }

                // Do not discard if this is part of minimal spanning tree
                if (shouldDiscard) {
                    if (minTreeMap.hasConnections(i)) {
                        const auto& minConns = minTreeMap.getConnections(i);
                        if (isInArray(minConns, conn)) {
                            shouldDiscard = false;
                        }
                    }

                    if (minTreeMap.hasConnections(conn)) {
                        const auto& minConns = minTreeMap.getConnections(conn);
                        if (isInArray(minConns, i)) {
                            shouldDiscard = false;
                        }
                    }
                }

                if (!shouldDiscard) {
                    // Insert if not self and not already included
                    auto exists = isInArray(system.connections, other.id);
                    if (!exists && system.id != other.id) {
                        system.connections.emplace_back(other.id);
                    }

                    exists = isInArray(system.connections, system.id);
                    if (!exists && system.id != other.id) {
                        other.connections.emplace_back(system.id);
                    }
                }
            }
        }
    }

    // Validate all systems have at least one connection
    for (auto& system : systems) {
        if (system.connections.empty()) {
            EXCEPTION("Generating connections failed, not all systems have connections");
        }
    }

    logger.info("Generated connections for {} systems", systems.size());

    // Index map converts a system id into an index
    std::unordered_map<std::string, size_t> indexes;
    indexes.reserve(systems.size());
    for (size_t i = 0; i < systems.size(); i++) {
        indexes.emplace(systems.at(i).id, i);
    }

    // Connection map is a map of integer -> list of integers
    std::unordered_map<size_t, std::vector<size_t>> connections;
    for (size_t i = 0; i < systems.size(); i++) {
        auto& system = systems.at(i);
        connections[i] = {};
        for (const auto& otherId : system.connections) {
            connections[i].push_back(indexes[otherId]);
        }
    }

    fillGalaxyRegions(galaxyId, positions, connections, systems);
    findFactionHomes(galaxyId, systems);
    fillGalaxyFactions(galaxyId, positions, connections, systems);

    for (size_t i = 0; i < systems.size(); i++) {
        fillSystemPlanets(galaxyId, systems.at(i));

        const auto key = fmt::format("{}/{}", galaxyId, systems.at(i).id);
        db.put<SystemData>(key, systems.at(i));
    }

    logger.info("Created {} systems in galaxy {}", systems.size(), galaxyId);
}

void Generator::createGalaxySectors(const std::string& galaxyId) {
    const auto galaxy = db.get<GalaxyData>(galaxyId);

    size_t total{0};
    auto it = db.seek<SystemData>(fmt::format("{}/", galaxyId));
    while (it.next()) {
        total += createGalaxySectors(galaxy, it.value());
    }

    logger.info("Created {} sectors in galaxy {}", total, galaxyId);
}

size_t Generator::createGalaxySectors(const GalaxyData& galaxy, const SystemData& system) {
    Rng rng{system.seed};

    size_t total{0};

    const auto planets = db.seekAll<PlanetData>(fmt::format("{}/{}/", galaxy.id, system.id), 0);

    // Find the radius of the entire solar system
    auto systemRadius{0.0f};
    for (const auto& planet : planets) {
        systemRadius = std::max<float>(glm::length(planet.pos), systemRadius);
    }
    // Add some extra space beyond planet orbits
    systemRadius += 25.0f;

    // Utility to find a random position within the system
    std::vector<Vector2> positions;
    const auto findPosition = [&]() -> std::optional<Vector2> {
        auto tries = 100;
        while (tries-- > 0) {
            const auto dist = randomReal<float>(rng, 5.0f, systemRadius);
            const auto angle = randomReal<float>(rng, 0.0f, 360.0f);
            const auto test = glm::rotate(Vector2{dist, 0.0f}, glm::radians(angle));

            auto success{true};
            for (const auto& pos : positions) {
                if (glm::distance(test, pos) < 2.0f) {
                    success = false;
                    break;
                }
            }

            if (success) {
                return test;
            }
        }
        return std::nullopt;
    };

    for (const auto& [name, type] : sectorTypes) {
        if (!type.checkConditions(rng, galaxy, system, planets)) {
            continue;
        }

        const auto count = randomInt<int>(rng, type.minCount, type.maxCount);

        for (int i = 0; i <= count; i++) {
            const auto pos = findPosition();
            if (!pos) {
                return total;
            }

            SectorData sector{};
            sector.id = uuid();
            sector.name = randomNameGenerator(rng);
            sector.pos = *pos;
            sector.galaxyId = galaxy.id;
            sector.systemId = system.id;
            sector.seed = randomSeed(rng);
            sector.icon = type.mapIcon;
            sector.type = name;

            const auto key = fmt::format("{}/{}/{}", galaxy.id, system.id, sector.id);
            db.put<SectorData>(key, sector);

            // Mark this sector as a starting location
            StartingLocationData startingLocation{};
            startingLocation.galaxyId = galaxy.id;
            startingLocation.systemId = system.id;
            startingLocation.sectorId = sector.id;
            db.put<StartingLocationData>(uuid(), startingLocation);
        }

        total += count;
    }

    return total;
}

void Generator::fillGalaxyRegions(const std::string& galaxyId, const std::vector<Vector2>& positions,
                                  const std::unordered_map<size_t, std::vector<size_t>>& connections,
                                  std::vector<SystemData>& systems) {

    auto regions = db.seekAll<RegionData>(fmt::format("{}/", galaxyId));

    auto floodFill = FloodFill{};

    for (size_t i = 0; i < regions.size(); i++) {
        floodFill.addStartPoint(regions.at(i).pos, i);
    }

    for (size_t i = 0; i < positions.size(); i++) {
        floodFill.addPosition(positions.at(i), connections.at(i));
    }

    floodFill.calculate();

    // Go through the generated flood map and associate systems
    for (size_t i = 0; i < floodFill.size(); i++) {
        const auto res = floodFill.get(i);
        systems.at(res.index).regionId = regions.at(res.point).id;
    }

    // Validate all systems have regions associated
    for (auto& system : systems) {
        if (system.regionId.empty()) {
            EXCEPTION("Generating regions failed, not all systems have a region associated");
        }
    }

    logger.info("Associated regions with {} systems", systems.size());
}

void Generator::findFactionHomes(const std::string& galaxyId, std::vector<SystemData>& systems) {
    auto factions = db.seekAll<FactionData>("");
    if (factions.empty()) {
        EXCEPTION("Can not generate galaxy, no factions in the database");
    }

    const auto galaxy = db.get<GalaxyData>(galaxyId);

    // Random generator specific to generating faction homes
    Rng rng{galaxy.seed + 30};

    // Generate random positions across the galaxy
    const auto positions =
        randomCirclePositions(rng, options.galaxyWidthMax * 0.8f, factions.size(), options.galaxyFactionDistanceMin);

    for (size_t i = 0; i < factions.size(); i++) {
        const auto& pos = positions.at(i);
        const auto& system = getNearestSystem(systems, pos);

        factions.at(i).homeGalaxyId = galaxyId;
        factions.at(i).homeSystemId = system.id;
        db.put<FactionData>(factions.at(i).id, factions.at(i));
    }

    logger.info("Updated faction homes");
}

void Generator::fillGalaxyFactions(const std::string& galaxyId, const std::vector<Vector2>& positions,
                                   const std::unordered_map<size_t, std::vector<size_t>>& connections,
                                   std::vector<SystemData>& systems) {
    // Index map converts a system id into an index
    std::unordered_map<std::string, size_t> indexes;
    indexes.reserve(systems.size());
    for (size_t i = 0; i < systems.size(); i++) {
        indexes.emplace(systems.at(i).id, i);
    }

    auto factions = db.seekAll<FactionData>("");
    if (factions.empty()) {
        EXCEPTION("Can not generate galaxy, no factions in the database");
    }

    auto floodFill = FloodFill{};

    for (size_t i = 0; i < factions.size(); i++) {
        const auto& system = systems.at(indexes.at(factions.at(i).homeSystemId));
        floodFill.addStartPoint(system.pos, i);
    }

    for (size_t i = 0; i < positions.size(); i++) {
        floodFill.addPosition(positions.at(i), connections.at(i));
    }

    floodFill.calculate();

    std::unordered_map<const FactionData*, size_t> counts{};
    size_t total{0};

    // Go through the generated flood map and associate systems
    for (size_t i = 0; i < floodFill.size(); i++) {
        const auto& res = floodFill.get(i);
        const auto& faction = factions.at(res.point);

        if (counts.find(&faction) == counts.end()) {
            counts.emplace(&faction, 0);
        }

        auto count = counts.find(&faction);
        if (count->second < 120) {
            systems.at(res.index).factionId = faction.id;
            ++count->second;
            ++total;
        }
    }

    logger.info("Associated factions with {} systems", total);
}

void Generator::fillSystemPlanets(const std::string& galaxyId, const SystemData& system) {
    const auto galaxy = db.get<GalaxyData>(galaxyId);
    const auto planetTypes = assetsManager.getPlanetTypes().findAll();
    if (planetTypes.empty()) {
        EXCEPTION("Can not generate galaxy, no planet types in the assets");
    }

    // Random generator specific to generating faction homes
    Rng rng{galaxy.seed + 40};

    float orbitDistance = 0.0f;
    const auto totalPlanets = randomInt<size_t>(rng, options.systemPlanetsMin, options.systemPlanetsMax);

    for (size_t p = 0; p < totalPlanets; p++) {
        PlanetData planet{};
        planet.id = uuid();
        planet.name = fmt::format("{} {}", system.name, p);
        planet.galaxyId = galaxyId;
        planet.systemId = system.id;
        planet.parentId = std::nullopt;
        planet.type = randomPick(rng, planetTypes);
        planet.radius = randomReal<float>(rng, 1.0f, 2.0f);

        float moonOrbitDistance{0.0f};
        std::vector<PlanetData> moons{};

        const auto totalMoons = randomInt<size_t>(rng, options.planetMoonsMin, options.planetMoonsMax);
        moons.reserve(totalMoons);

        for (size_t m = 0; m < totalMoons; m++) {
            moons.emplace_back();
            auto& moon = moons.back();

            moon.id = uuid();
            moon.name = fmt::format("{} {} - {}", system.name, p, m);
            moon.galaxyId = galaxyId;
            moon.systemId = system.id;
            moon.parentId = planet.id;
            moon.type = randomPick(rng, planetTypes);
            moon.radius = randomReal<float>(rng, 0.2f, 0.6f);

            auto randDistance = randomReal<float>(rng, options.moonDistanceMin, options.moonDistanceMax);
            moonOrbitDistance += randDistance;
            moon.pos = glm::rotate(Vector2{moonOrbitDistance, 0.0f}, randomReal<float>(rng, 0.0f, 6.28318530718f));
        }

        const auto randDistance = randomReal<float>(rng, options.planetDistanceMin, options.planetDistanceMax);
        planet.pos = glm::rotate(Vector2{orbitDistance, 0.0f}, randomReal<float>(rng, 0.0f, 6.28318530718f));

        for (auto& moon : moons) {
            moon.pos += planet.pos;

            db.put<PlanetData>(fmt::format("{}/{}/{}", galaxyId, system.id, moon.id), moon);
        }

        orbitDistance += moonOrbitDistance;
        db.put<PlanetData>(fmt::format("{}/{}/{}", galaxyId, system.id, planet.id), planet);
    }
}
