#include "ServiceSectors.hpp"
#include "../../Utils/Random.hpp"
#include "../../Utils/StringUtils.hpp"

#define CMP "ServiceSectors"

using namespace Engine;

ServiceSectors::ServiceSectors(const Config& config, AssetManager& assetManager, Engine::Database& db)
    : config(config), assetManager(assetManager), db(db) {
}

void ServiceSectors::tick() {
}

void ServiceSectors::generate() {
    const auto galaxies = db.seek<GalaxyData>("");
    for (const auto& galaxy : galaxies) {
        generate(galaxy.id);
    }
}

std::optional<SectorData> ServiceSectors::find(const std::string& galaxyId, const std::string& systemId,
                                               const std::string& sectorId) {
    return db.get<SectorData>(fmt::format("{}/{}/{}", galaxyId, systemId, sectorId));
}

void ServiceSectors::generate(const std::string& galaxyId) {
    Log::i(CMP, "Generating sectors for galaxy: '{}' ...", galaxyId);

    const auto systems = db.seek<SystemData>("");
    for (const auto& system : systems) {
        generate(galaxyId, system.id);
    }
}

void ServiceSectors::generate(const std::string& galaxyId, const std::string& systemId) {
    const auto galaxyOpt = db.get<GalaxyData>(galaxyId);
    if (!galaxyOpt) {
        EXCEPTION("No such galaxy: '{}'", galaxyId);
    }

    const auto& galaxy = galaxyOpt.value();

    const auto systemOpt = db.get<SystemData>(fmt::format("{}/{}", galaxyId, systemId));
    if (!systemOpt) {
        EXCEPTION("No such system: '{}/{}'", galaxyId, systemId);
    }

    const auto& options = config.generator;
    const auto& system = systemOpt.value();

    const auto compoundPrefix = fmt::format("{}/{}/", galaxyId, systemId);
    const auto test = db.seek<SectorData>(compoundPrefix, 1);
    if (!test.empty()) {
        // Already generated
        return;
    }

    std::mt19937_64 rng(system.seed);

    auto assetPlanets = assetManager.findAll<AssetPlanet>();
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
    }

    // TODO
    SectorData sector;
    sector.id = uuid();
    sector.systemId = system.id;
    sector.galaxyId = system.galaxyId;
    sector.pos = Vector2{0.0f, 0.0f};
    sector.name = fmt::format("{} (Sector)", system.name);
    sector.generated = false;
    sector.seed = randomInt<uint64_t>(rng);

    createSector(sector);
}

void ServiceSectors::createSectorPlanet(const SectorPlanetData& planet) {
    db.put(fmt::format("{}/{}/{}", planet.galaxyId, planet.systemId, planet.id), planet);
}

void ServiceSectors::createSector(const SectorData& sector) {
    db.put(fmt::format("{}/{}/{}", sector.galaxyId, sector.systemId, sector.id), sector);
}
