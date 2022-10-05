#include "Generator.hpp"
#include "../Math/DelaunayTriangulation.hpp"
#include "../Math/FloodFill.hpp"
#include "../Math/GalaxyDistribution.hpp"
#include "../Math/MinimumSpanningTree.hpp"
#include "../Math/Random.hpp"
#include "../Utils/Random.hpp"

#define CMP "Generator"

using namespace Engine;

Generator::Generator(const Config& config, TransactionalDatabase& db, World& world) :
    config(config), db(db), world(world) {
}

void Generator::generate(uint64_t seed) {
    if (isGenerated()) {
        return;
    }

    SaveFileData data{};
    data.seed = seed;
    data.generated = false;
    db.put("", data);

    generateGalaxy(seed);

    data.generated = true;
    db.put("", data);
}

bool Generator::isGenerated() {
    auto found = db.get<SaveFileData>("");
    if (!found) {
        return false;
    }

    if (!found->generated) {
        EXCEPTION("Save file was generated but got corrupted during creation");
    }

    return true;
}

void Generator::generateGalaxy(uint64_t seed) {
    Log::i(CMP, "Generating galaxies ...");

    std::mt19937_64 rng(seed);

    GalaxyData galaxy;
    galaxy.id = uuid();
    galaxy.name = fmt::format("{} Galaxy", randomName(rng));
    galaxy.pos = {0.0f, 0.0f};
    galaxy.seed = randomInt<uint64_t>(rng);

    world.galaxies.create(galaxy);

    auto regions = generateGalaxyRegions(galaxy);

    auto systems = generateGalaxySystems(galaxy, regions);

    generateSectors(galaxy, systems);
}

std::vector<RegionData> Generator::generateGalaxyRegions(const GalaxyData& galaxy) {
    static const std::vector<std::string> regionSuffixes = {
        "Region", "District", "Province", "Place",  "Territory", "Zone",    "Division",
        "Domain", "Expanse",  "Realm",    "Sphere", "Vicinity",  "Enclave", "Area",
    };

    Log::i(CMP, "Generating regions for galaxy: '{}' ...", galaxy.id);

    const auto& options = config.generator;

    std::mt19937_64 rng(galaxy.seed);

    const auto maxRange = options.galaxyWidth * 0.9f - options.regionDistance / 2.0f;

    std::vector<Vector2> positions =
        randomCirclePositions(rng, maxRange, options.regionMaxCount, options.regionDistance);

    const auto randomSuffix = [&]() { return regionSuffixes.at(randomInt<size_t>(rng, 0, regionSuffixes.size() - 1)); };

    std::vector<RegionData> regions;
    regions.reserve(positions.size());

    for (const auto& pos : positions) {
        RegionData region;
        region.id = uuid();
        region.galaxyId = galaxy.id;
        region.seed = randomInt<uint64_t>(rng);
        region.name = fmt::format("{} {}", randomName(rng), randomSuffix());
        region.pos = pos;

        world.regions.create(region);

        regions.push_back(std::move(region));
    }

    Log::i(CMP, "Generated {} regions for galaxy '{}'", regions.size(), galaxy.name);

    return regions;
}

std::vector<SystemData> Generator::generateGalaxySystems(const GalaxyData& galaxy,
                                                         const std::vector<RegionData>& regions) {
    const auto& options = config.generator;

    Log::i(CMP, "Generating systems for galaxy: '{}' ...", galaxy.id);

    // Not optimized but the number of regions will be very low.
    // Should be around 18
    const auto getNearestRegion = [&](const Vector2& pos) {
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

        assert(ptr != nullptr);
        return *ptr;
    };

    std::vector<SystemData> systems;
    systems.reserve(options.totalSystems);

    const auto getNearestSystem = [&](const Vector2& pos) -> SystemData& {
        SystemData* ptr = nullptr;
        for (auto& system : systems) {
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

        assert(ptr != nullptr);
        return *ptr;
    };

    std::mt19937_64 rng(galaxy.seed);

    GalaxyDistribution distributor(options.galaxyWidth);

    std::unordered_set<std::string> namesTaken;
    namesTaken.reserve(options.totalSystems);

    std::vector<Vector2> positions;

    for (auto i = 0; i < options.totalSystems; i++) {
        const auto pos = distributor(rng);
        // const auto region = getNearestRegion(pos);

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

        SystemData system;
        system.id = uuid();
        system.galaxyId = galaxy.id;
        // system.regionId = region.id;
        system.name = name;
        system.pos = pos;
        system.seed = randomInt<uint64_t>(rng);

        systems.push_back(system);
        positions.push_back(system.pos);
    }

    MinimumSpanningTree connectionSolver(positions);
    const auto minTreeMap = connectionSolver.solve();

    DelaunayTriangulation delaunay(positions);
    const auto triangulationMap = delaunay.solve();

    for (size_t i = 0; i < systems.size(); i++) {
        auto& system = systems.at(i);

        const auto connections = triangulationMap.find(i);
        if (connections != triangulationMap.end()) {
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

                system.connections.push_back(other.id);
                other.connections.push_back(system.id);
            }
        }
    }

    std::unordered_map<std::string, size_t> systemIdToIndex;
    systemIdToIndex.reserve(systems.size());
    for (size_t i = 0; i < systems.size(); i++) {
        systemIdToIndex.insert(std::make_pair(systems.at(i).id, i));
    }

    std::unordered_map<size_t, std::vector<size_t>> connections;
    connections.reserve(systems.size());

    for (size_t i = 0; i < systems.size(); i++) {
        auto it = connections.insert(std::make_pair(i, std::vector<size_t>{})).first;

        const auto& conns = systems.at(i).connections;
        it->second.reserve(conns.size());

        for (const auto& otherId : conns) {
            const auto idx = systemIdToIndex.at(otherId);
            it->second.push_back(idx);
        }
    }

    // Flood fill regions
    {
        std::unordered_map<size_t, Vector2> regionPoints;
        regionPoints.reserve(regions.size());
        for (size_t i = 0; i < regions.size(); i++) {
            regionPoints.insert(std::make_pair(i, regions.at(i).pos));
        }

        FloodFill(positions, connections, regionPoints)([&](size_t systemIdx, size_t regionIdx) {
            // Assign system to the region
            systems.at(systemIdx).regionId = regions.at(regionIdx).id;
        });
    }

    std::vector<FactionData> factions;
    std::unordered_map<size_t, Vector2> factionsPoints;

    // Generate factions
    {
        const auto maxRange = options.galaxyWidth * 0.9f;
        std::vector<Vector2> factionPositions =
            randomCirclePositions(rng, maxRange, options.factionMaxCount, options.factionDistance);

        factions.reserve(factionPositions.size());

        for (size_t i = 0; i < factionPositions.size(); i++) {
            const auto& pos = factionPositions.at(i);

            factions.emplace_back();
            auto& faction = factions.back();
            faction.id = uuid();
            faction.name = "Faction " + faction.id;
            faction.color = randomReal(rng, 0.0f, 360.0f);

            factionsPoints.insert(std::make_pair(i, pos));
        }
    }

    // Flood fill factions
    {
        FloodFill(positions, connections, factionsPoints)([&](size_t systemIdx, size_t factionIdx) {
            // Assign system to the region
            systems.at(systemIdx).factionId = factions.at(factionIdx).id;
        });
    }

    for (const auto& faction : factions) {
        world.factions.create(faction);
    }

    for (const auto& system : systems) {
        world.systems.create(system);
    }

    Log::i(CMP, "Generated {} systems for galaxy: '{}'", systems.size(), galaxy.name);

    return systems;
}

void Generator::generateSectors(const GalaxyData& galaxy, const std::vector<SystemData>& systems) {
    for (const auto& system : systems) {
        generateSectors(galaxy, system);
    }
}

void Generator::generateSectors(const GalaxyData& galaxy, const SystemData& system) {
    std::mt19937_64 rng(system.seed);

    /*auto assetPlanets = assetManager.findAll<AssetPlanet>();
    if (assetPlanets.empty()) {
        EXCEPTION("There are no planets found in assets");
    }

    const auto nextOrbit = [&](float& offset, float max) {
        std::uniform_real_distribution<float> radiusDist{offset, offset + max};
        std::uniform_real_distribution<float> angleDist{0.0f, 360.0f};
        Vector2 pos{radiusDist(rng), 0.0f};
        offset = pos.x + 5.0f;
        return glm::rotate(pos, glm::radians(angleDist(rng)));
    };

    float planetaryOrbitRadius = 20.0f;

    // Create planet and moons
    const auto totalPlanets = randomInt(rng, options.systemPlanetsMin, options.systemPlanetsMax);
    for (auto i = 0; i < totalPlanets; i++) {
        const auto totalMoons = randomInt(rng, options.planetMoonsMin, options.planetMoonsMax);

        SectorPlanetData planet{};
        planet.id = uuid();
        planet.name = fmt::format("{} {}", system.name, intToRomanNumeral(i + 1));
        planet.isMoon = false;
        planet.asset = assetPlanets.front();
        planet.systemId = system.id;
        planet.galaxyId = galaxy.id;

        // Create random moon orbits
        std::vector<Vector2> orbits;
        float moonOrbitRadius = 0.0f;
        if (totalMoons > 0) {
            moonOrbitRadius = 10.0f;
        }

        for (auto m = 0; m < totalMoons; m++) {
            orbits.push_back(nextOrbit(moonOrbitRadius, 5.0f));
        }

        planetaryOrbitRadius += moonOrbitRadius;

        // Create planet orbits
        for (auto m = 0; m < totalMoons; m++) {
            SectorPlanetData moon{};
            moon.id = uuid();
            moon.name = fmt::format("{} (Moon {})", planet.name, intToRomanNumeral(m + 1));
            moon.isMoon = true;
            moon.planet = planet.id;
            moon.pos = orbits.at(m);
            moon.asset = assetPlanets.front();
            moon.systemId = system.id;
            moon.galaxyId = galaxy.id;

            createSectorPlanet(moon);
        }

        // Calculate the orbit for this planet
        planet.pos = nextOrbit(planetaryOrbitRadius, 5.0f);

        createSectorPlanet(planet);

        // Advance to the next orbit
        planetaryOrbitRadius += moonOrbitRadius + 15.0f;
    }*/

    // TODO
    SectorData sector;
    sector.id = uuid();
    sector.systemId = system.id;
    sector.galaxyId = system.galaxyId;
    sector.pos = Vector2{0.0f, 0.0f};
    sector.name = fmt::format("{} (Sector)", system.name);
    sector.generated = false;
    sector.seed = randomInt<uint64_t>(rng);

    world.sectors.create(sector);
}
