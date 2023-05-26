#include "generator.hpp"
#include "../math/delaunay_triangulation.hpp"
#include "../math/flood_fill.hpp"
#include "../math/galaxy_distribution.hpp"
#include "../math/minimum_spanning_tree.hpp"
#include "../math/random.hpp"
#include "../utils/name_generator.hpp"
#include "../utils/random.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

void Generator::generate(uint64_t seed) {
    logger.info("Generating universe with seed: {}", seed);
}

// Not optimized but the number of regions will be very low.
// Should be around 18 regions.
/*static const RegionData& getNearestRegion(const std::vector<RegionData>& regions, const Vector2& pos) {
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
};*/

/*static const SystemData& getNearestSystem(const std::vector<SystemData>& systems, const Vector2& pos) {
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
};*/

/*static std::vector<Vector2> getSystemPositions(const std::vector<SystemData>& systems) {
    std::vector<Vector2> positions;
    positions.reserve(systems.size());

    for (const auto& system : systems) {
        positions.push_back(system.pos);
    }

    return positions;
}*/

// using SystemIndexMap = std::unordered_map<std::string, size_t>;

/*static SystemIndexMap getSystemIdToIndexMap(const std::vector<SystemData>& systems) {
    SystemIndexMap map;
    map.reserve(systems.size());
    for (size_t i = 0; i < systems.size(); i++) {
        map.insert(std::make_pair(systems.at(i).id, i));
    }

    return map;
}*/

// using SystemConnectionMap = std::unordered_map<size_t, std::vector<size_t>>;

/*static SystemConnectionMap getSystemConnectionMap(const std::vector<SystemData>& systems, const SystemIndexMap& index)
{ SystemConnectionMap connections; connections.reserve(systems.size());

    for (size_t i = 0; i < systems.size(); i++) {
        auto it = connections.insert(std::make_pair(i, std::vector<size_t>{})).first;

        const auto& conns = systems.at(i).connections;
        it->second.reserve(conns.size());

        for (const auto& otherId : conns) {
            const auto idx = index.at(otherId);
            it->second.push_back(idx);
        }
    }

    return connections;
}*/

Generator::Generator(const Config& config, World& world) : config{config}, world{world} {
}

/*void Generator::generate(const uint64_t seed) {
    // Won't do an actual create if the save data already exists
    auto index = world.getIndex();

    if (index.seed != seed) {
        index = world.updateIndex([=](IndexData& index) { index.seed = seed; });
    }

    // Was the universe created?
    if (!index.galaxyId.empty()) {
        logger.info("Universe already generated");
        return;
    }

    // Generate the main galaxy
    std::mt19937_64 rng{index.seed};
    const auto galaxyId = generateGalaxy(rng, randomInt<uint64_t>(rng));
    index = world.updateIndex([=](IndexData& index) { index.galaxyId = galaxyId; });

    generateGalaxyRegions(index.galaxyId);
    generateGalaxySystems(index.galaxyId);
    generateGalaxyFactions(index.galaxyId);
    generateGalaxySectors(index.galaxyId);

    logger.info("Universe generated");
}*/

/*std::string Generator::generateGalaxy(Rng& rng, const uint64_t seed) {
    logger.info("Generating galaxy");

    GalaxyData galaxy;
    galaxy.id = uuid();
    galaxy.name = fmt::format("{} Galaxy", randomName(rng));
    galaxy.pos = {0.0f, 0.0f};
    galaxy.seed = seed;

    world.galaxies.create(galaxy);

    return galaxy.id;
}*/

/*void Generator::generateGalaxyRegions(const std::string& galaxyId) {
    static const std::vector<std::string> regionSuffixes = {
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

    logger.info("Generating regions for galaxy: '{}' ...", galaxyId);

    const auto galaxy = world.galaxies.get(galaxyId);
    std::mt19937_64 rng{galaxy.seed + 10};

    const auto maxRange = config.generator.galaxyWidth * 0.4f - config.generator.regionDistance / 2.0f;

    std::vector<Vector2> positions =
        randomCirclePositions(rng, maxRange, config.generator.regionMaxCount, config.generator.regionDistance);

    const auto randomSuffix = [&]() { return regionSuffixes.at(randomInt<size_t>(rng, 0, regionSuffixes.size() - 1)); };

    for (const auto& pos : positions) {
        RegionData region;
        region.id = uuid();
        region.galaxyId = galaxy.id;
        region.seed = randomInt<uint64_t>(rng);
        region.name = fmt::format("{} {}", randomName(rng), randomSuffix());
        region.pos = pos;

        world.regions.create(region);
    }

    logger.info("Generated {} regions for galaxy '{}'", positions.size(), galaxy.name);
}*/

/*void Generator::generateGalaxySystems(const std::string& galaxyId) {
    logger.info("Generating systems for galaxy: '{}' ...", galaxyId);

    const auto galaxy = world.galaxies.get(galaxyId);
    const auto regions = world.regions.getForGalaxy(galaxyId);
    const auto factions = world.factions.get();

    if (regions.empty()) {
        EXCEPTION("Galaxy '{}' has no regions", galaxyId);
    }

    std::mt19937_64 rng{galaxy.seed + 20};

    std::vector<SystemData> systems;
    systems.resize(config.generator.totalSystems);

    generateSystemDistribution(rng, galaxy.id, systems, regions);
    connectSystems(rng, systems);
    floodFillRegions(rng, systems, regions);

    for (auto& system : systems) {
        world.systems.create(system);
    }
}*/

/*void Generator::generateGalaxyFactions(const std::string& galaxyId) {
    logger.info("Generating factions for galaxy: '{}' ...", galaxyId);

    const auto galaxy = world.galaxies.get(galaxyId);
    auto systems = world.systems.getForGalaxy(galaxyId);

    std::mt19937_64 rng{galaxy.seed + 30};

    const auto maxRange = config.generator.galaxyWidth * 0.8f;
    std::vector<Vector2> factionPositions =
        randomCirclePositions(rng, maxRange, config.generator.factionMaxCount, config.generator.factionDistance);

    std::vector<FactionData> factions;
    for (size_t i = 0; i < factionPositions.size(); i++) {
        const auto& sector = getNearestSystem(systems, factionPositions.at(i));

        factions.emplace_back();
        auto& faction = factions.back();
        faction.id = uuid();
        faction.name = "Faction " + faction.id;
        faction.color = (i / static_cast<float>(factionPositions.size())) * 360.0f;
        faction.homeSectorId = sector.id;

        world.factions.create(faction);
    }

    floodFillFactions(rng, systems, factions);

    for (const auto& system : systems) {
        if (system.factionId) {
            world.systems.update(system);
        }
    }
}*/

/*void Generator::generateGalaxySectors(const std::string& galaxyId) {
    logger.info("Generating sectors for galaxy: '{}' ...", galaxyId);

    const auto galaxy = world.galaxies.get(galaxyId);
    const auto systems = world.systems.getForGalaxy(galaxyId);

    for (const auto& system : systems) {
        std::mt19937_64 rng{system.seed};
        populateSystem(rng, galaxy, system);
    }
}*/

/*void Generator::generateSystemDistribution(Generator::Rng& rng, const std::string& galaxyId,
                                           std::vector<SystemData>& systems, const std::vector<RegionData>& regions) {
    logger.info("Calculating galaxy distribution for {} systems...", systems.size());

    GalaxyDistribution distributor{config.generator.galaxyWidth};

    std::unordered_set<std::string> namesTaken;
    namesTaken.reserve(systems.size());

    float radius = 0.0f;

    for (auto i = 0; i < config.generator.totalSystems; i++) {
        const auto pos = distributor(rng);

        // System name
        std::string name;
        while (true) {
            const auto test = randomName(rng);
            if (namesTaken.find(test) == namesTaken.end()) {
                name = test;
                namesTaken.insert(test);
                break;
            }
        }

        auto& system = systems.at(i);
        system.id = uuid();
        system.galaxyId = galaxyId;
        system.name = name;
        system.pos = pos;
        system.seed = randomInt<uint64_t>(rng);

        if (const auto dist = glm::length(pos); dist > radius) {
            radius = dist;
        }
    }

    logger.info("Galaxy has radius of: {} configured: {}", radius, config.generator.galaxyWidth);
}*/

/*void Generator::connectSystems(Generator::Rng& rng, std::vector<SystemData>& systems) {
    logger.info("Creating connections for {} systems...", systems.size());

    const auto positions = getSystemPositions(systems);

    const auto minTreeMap = minimumSpanningTree(positions);
    if (minTreeMap.empty()) {
        EXCEPTION("Failed to generate minimum spanning tree");
    }

    const auto triangulationMap = delaunayTriangulation(positions);
    if (triangulationMap.empty()) {
        EXCEPTION("Failed to generate delaunay triangulation");
    }

    for (size_t i = 0; i < systems.size(); i++) {
        auto& system = systems.at(i);

        const auto connections = triangulationMap.find(i);
        if (connections != triangulationMap.end()) {
            if (connections->second.empty()) {
                EXCEPTION("Delaunary triangulation failed, system '{}' has no connection", system.id);
            }

            for (const auto conn : connections->second) {
                auto& other = systems.at(conn);

                bool shouldDiscard = false;

                // Discard connection if it is too far from each other
                const auto dist = glm::distance(other.pos, system.pos);
                if (dist > 40.0f) {
                    shouldDiscard = true;
                }

                // Discard connection randomly
                if (randomReal(rng, 0.0f, 1.0f) < 0.6f) {
                    shouldDiscard = true;
                }

                // Do not discard if this is part of minimal spanning tree
                if (shouldDiscard) {
                    if (const auto mit = minTreeMap.find(i); mit != minTreeMap.end()) {
                        if (std::find(mit->second.begin(), mit->second.end(), conn) != mit->second.end()) {
                            shouldDiscard = false;
                        }
                    }

                    if (const auto mit = minTreeMap.find(conn); mit != minTreeMap.end()) {
                        if (std::find(mit->second.begin(), mit->second.end(), i) != mit->second.end()) {
                            shouldDiscard = false;
                        }
                    }
                }

                if (shouldDiscard) {
                    continue;
                }

                // Insert if not self and not already included
                auto exists = std::find(system.connections.begin(), system.connections.end(), other.id);
                if (exists == system.connections.end() && system.id != other.id) {
                    system.connections.push_back(other.id);
                }

                // Insert if not self and not already included
                exists = std::find(other.connections.begin(), other.connections.end(), system.id);
                if (exists == other.connections.end() && system.id != other.id) {
                    other.connections.push_back(system.id);
                }
            }
        }
    }

    // Validate all systems have at least one connection
    for (const auto& system : systems) {
        if (system.connections.empty()) {
            EXCEPTION("Genearting connections failed, system '{}' has no connection", system.id);
        }
    }
}*/

/*void Generator::floodFillRegions(Generator::Rng& rng, std::vector<SystemData>& systems,
                                 const std::vector<RegionData>& regions) {

    logger.info("Flood filling regions for {} systems with {} regions ...", systems.size(), regions.size());

    const auto positions = getSystemPositions(systems);
    const auto indexes = getSystemIdToIndexMap(systems);
    const auto connections = getSystemConnectionMap(systems, indexes);

    std::unordered_map<size_t, Vector2> regionPoints;
    regionPoints.reserve(regions.size());
    for (size_t i = 0; i < regions.size(); i++) {
        regionPoints.insert(std::make_pair(i, regions.at(i).pos));
    }

    floodFill(positions, connections, regionPoints, [&](size_t systemIdx, size_t regionIdx) {
        // Assign system to the region
        systems.at(systemIdx).regionId = regions.at(regionIdx).id;
    });
}*/

/*void Generator::floodFillFactions(Generator::Rng& rng, std::vector<SystemData>& systems,
                                  const std::vector<FactionData>& factions) {

    logger.info("Flood filling factions for {} systems with {} factions ...", systems.size(), factions.size());

    const auto positions = getSystemPositions(systems);
    const auto indexes = getSystemIdToIndexMap(systems);
    const auto connections = getSystemConnectionMap(systems, indexes);

    std::unordered_map<size_t, Vector2> factionPoints;
    std::unordered_map<size_t, size_t> factionSystemsCount;
    std::unordered_map<size_t, size_t> factionSystemsMaxCount;

    factionPoints.reserve(factions.size());
    for (size_t i = 0; i < factions.size(); i++) {
        auto& faction = factions.at(i);
        const auto home =
            std::find_if(systems.begin(), systems.end(), [&](const auto& s) { return s.id == faction.homeSectorId; });

        factionPoints.insert(std::make_pair(i, home->pos));
        factionSystemsCount.insert(std::make_pair(i, 0));

        std::uniform_int_distribution<int> maxCountDist{config.generator.factionSystemsMin,
                                                        config.generator.factionSystemsMax};
        factionSystemsMaxCount.insert(std::make_pair(i, maxCountDist(rng)));
    }

    floodFill(positions, connections, factionPoints, [&](size_t systemIdx, size_t factionIdx) {
        // Assign system to the faction
        auto& currentCount = factionSystemsCount.at(factionIdx);
        if (currentCount + 1 <= factionSystemsMaxCount.at(factionIdx)) {
            systems.at(systemIdx).factionId = factions.at(factionIdx).id;
            currentCount += 1;
        }
    });
}*/

/*void Generator::populateSystem(Rng& rng, const GalaxyData& galaxy, const SystemData& system) {
    populateSystemPlanets(rng, galaxy, system);

    const auto planetaryBodies = world.systems.getPlanetaryBodies(galaxy.id, system.id);

    for (const auto& planet : planetaryBodies) {
        if (planet.isMoon) {
            continue;
        }

        if (randomReal(rng, 0.0f, 1.0f) > 0.5f) {
            continue;
        }

        const auto offset = glm::rotate(Vector2{2.0f, 0.0f}, glm::radians(randomReal(rng, 0.0f, 1.0f)));

        SectorData sector;
        sector.id = uuid();
        sector.systemId = system.id;
        sector.galaxyId = galaxy.id;
        sector.pos = offset + planet.pos;
        sector.name = fmt::format("{} (Sector)", planet.name);
        sector.generated = false;
        sector.seed = randomInt<uint64_t>(rng);

        world.sectors.create(sector);
    }
}*/

/*void Generator::populateSystemPlanets(Rng& rng, const GalaxyData& galaxy, const SystemData& system) {
    const auto totalPlanets = randomInt(rng, config.generator.systemPlanetsMin, config.generator.systemPlanetsMax);
    float sunOrbitDistance = config.generator.planetStartDistance;

    for (int i = 0; i < totalPlanets; i++) {
        const auto totalMoons = randomInt(rng, config.generator.planetMoonsMin, config.generator.planetMoonsMax);
        float moonsOrbitDistance = 0.0f;

        PlanetaryBodyData planet{};
        planet.id = uuid();
        planet.galaxyId = galaxy.id;
        planet.systemId = system.id;
        planet.isMoon = false;
        planet.name = fmt::format("{} {}", system.name, i + 1);

        std::vector<PlanetaryBodyData> moons;

        for (int j = 0; j < totalMoons; j++) {
            moons.emplace_back();
            PlanetaryBodyData& moon = moons.back();

            moon.id = uuid();
            moon.galaxyId = galaxy.id;
            moon.systemId = system.id;
            moon.isMoon = true;
            moon.parent = planet.id;
            moon.name = fmt::format("{} - {}", planet.name, j + 1);

            if (j == 0) {
                moonsOrbitDistance = config.generator.moonStartDistance;
            } else {
                moonsOrbitDistance +=
                    randomReal(rng, config.generator.moonDistanceMin, config.generator.moonDistanceMax);
            }
            moon.pos = Vector2{moonsOrbitDistance, 0.0f};
        }

        sunOrbitDistance += moonsOrbitDistance;
        planet.pos = glm::rotate(Vector2{sunOrbitDistance, 0.0f}, glm::radians(randomReal(rng, 0.0f, 360.0f)));
        sunOrbitDistance += moonsOrbitDistance;
        sunOrbitDistance += randomReal(rng, config.generator.planetDistanceMin, config.generator.planetDistanceMax);

        for (auto& moon : moons) {
            moon.pos = glm::rotate(moon.pos, glm::radians(randomReal(rng, 0.0f, 360.0f))) + planet.pos;

            world.systems.create(moon);
        }

        world.systems.create(planet);
    }
}*/
