#include "Generator.hpp"
#include "../Utils/Random.hpp"

#define CMP "Generator"

using namespace Scissio;

static const std::vector<std::string> regionSuffixes = {
    "Region", "District", "Province", "Place",  "Territory", "Zone",    "Division",
    "Domain", "Expanse",  "Realm",    "Sphere", "Vicinity",  "Enclave", "Area",
};

class GalaxyDistribution {
public:
    explicit GalaxyDistribution(const float maxWidth)
        : maxWidth(maxWidth), distArmIndex(0, ARMS_COUNT), distArmDistance(0.01f, 1.0f),
          distArmOffset(-ARM_ANGLE_HALF, ARM_ANGLE_HALF), distGridOffset(-0.1f, 0.1f) {
    }

    [[nodiscard]] Vector2 operator()(std::mt19937_64& rng) {
        Vector2i fixed;

        auto count = 5;
        while (count-- > 0) {
            const auto dist = distArmDistance(rng);
            auto angleOffset = (distArmOffset(rng) * (1.0f / dist));
            if (angleOffset > 0.0f) {
                angleOffset = std::pow(angleOffset, 2.0f);
            } else {
                angleOffset = -std::pow(-angleOffset, 2.0f);
            }
            auto angle = distArmIndex(rng) * ARM_ANGLE + angleOffset;

            angle = angle + dist * 2.0f;

            auto vec = Vector2{dist * maxWidth, 0.0f};
            const auto ca = std::cos(angle);
            const auto sa = std::sin(angle);
            vec = Vector2{ca * vec.x - sa * vec.y, sa * vec.x + ca * vec.y};

            fixed = Vector2i{vec};

            const auto test = *reinterpret_cast<uint64_t*>(&fixed.x);
            if (positions.find(test) == positions.end()) {
                positions.insert(test);
                break;
            }
        }

        Vector2 pos{fixed};
        pos = Vector2{pos.x + distGridOffset(rng), pos.y + distGridOffset(rng)};

        return pos;
    }

private:
    static constexpr auto ARMS_COUNT = 5;
    static constexpr auto ARM_ANGLE = static_cast<float>(2.0 * glm::pi<double>() / double(ARMS_COUNT));
    static constexpr auto ARM_ANGLE_HALF = ARM_ANGLE / 2.0f - ARM_ANGLE * 0.05f;

    float maxWidth;

    std::unordered_set<uint64_t> positions;
    std::uniform_int_distribution<int> distArmIndex;
    std::uniform_real_distribution<float> distArmDistance;
    std::uniform_real_distribution<float> distArmOffset;
    std::uniform_real_distribution<float> distGridOffset;
};

class Seeder {
public:
    Seeder(uint64_t seed) : r{seed} {
    }

    uint64_t operator()() {
        return dist(r);
    }

    std::mt19937_64& rng() {
        return r;
    }

private:
    std::mt19937_64 r;
    std::uniform_int_distribution<uint64_t> dist;
};

Generator::Generator(Database& db, Options options) : db(db), options(options) {
}

void Generator::generateWorld(uint64_t seed) {
    auto world = db.get<WorldData>("0");
    if (world.has_value()) {
        Log::i(CMP, "Not generating world, already exists");
        return;
    }

    Log::i(CMP, "Generating world ...");

    world = WorldData{};
    world.value().seed = seed;
    db.put("0", world.value());

    generateGalaxies(seed);
}

void Generator::generateGalaxies(uint64_t seed) {
    Log::i(CMP, "Generating galaxies ...");

    Seeder seeder(seed);

    GalaxyData galaxy;
    galaxy.id = uuid();
    galaxy.name = fmt::format("{} Galaxy", randomName(seeder.rng()));
    galaxy.pos = {0.0f, 0.0f};
    galaxy.seed = seed;

    db.put(galaxy.id, galaxy);

    generateRegions(seeder(), galaxy.id);
    generateSystems(seeder(), galaxy.id);
}

void Generator::generateRegions(uint64_t seed, const std::string& galaxyId) {
    const auto galaxy = db.get<GalaxyData>(galaxyId).value();
    Log::i(CMP, "Generating regions for galaxy '{}' ...", galaxy.name);

    Seeder seeder(seed);

    const auto maxRange = options.galaxyWidth - options.regionDistance / 2.0f;
    std::uniform_real_distribution<float> dist{-maxRange, maxRange};

    std::vector<Vector2> positions;
    auto tries = 20;

    while (tries > 0) {
        tries--;

        const auto test = Vector2{dist(seeder.rng()), dist(seeder.rng())};

        auto accepted = true;

        for (const auto& pos : positions) {
            if (glm::distance(test, pos) < options.regionDistance) {
                accepted = false;
                break;
            }
        }

        if (!accepted) {
            continue;
        }

        positions.push_back(test);
        tries = 5;
    }

    const auto randomSuffix = [&]() {
        std::uniform_int_distribution<size_t> dist(0, regionSuffixes.size() - 1);
        return regionSuffixes.at(dist(seeder.rng()));
    };

    for (const auto& pos : positions) {
        RegionData region;
        region.id = uuid();
        region.galaxyId = galaxyId;
        region.seed = seeder();
        region.name = fmt::format("{} {}", randomName(seeder.rng()), randomSuffix());
        region.pos = pos;

        db.put(fmt::format("{}/{}", galaxyId, region.id), region);
    }

    Log::i(CMP, "Generated {} regions for galaxy '{}'", positions.size(), galaxy.name);
}

void Generator::generateSystems(uint64_t seed, const std::string& galaxyId) {
    const auto galaxy = db.get<GalaxyData>(galaxyId).value();
    const auto regions = db.seek<RegionData>(fmt::format("{}/", galaxyId));

    Log::i(CMP, "Generating systems for galaxy '{}' ...", options.totalSystems, galaxy.name);

    if (regions.empty()) {
        EXCEPTION("Number of regions in galaxy '{}' must not be zero", galaxyId);
    }

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

    Seeder seeder(seed);

    GalaxyDistribution distributor(options.galaxyWidth);

    std::unordered_set<std::string> namesTaken;
    namesTaken.reserve(options.galaxyWidth);

    for (auto i = 0; i < options.totalSystems; i++) {
        const auto pos = distributor(seeder.rng());
        const auto region = getNearestRegion(pos);

        // System name
        std::string name;
        while (true) {
            const auto test = randomName(seeder.rng());
            if (namesTaken.find(test) == namesTaken.end()) {
                name = test;
                namesTaken.insert(test);
                break;
            }
        }

        SystemData system;
        system.id = uuid();
        system.galaxyId = galaxyId;
        system.regionId = region.id;
        system.name = name;
        system.pos = pos;
        system.seed = seeder();

        db.put(fmt::format("{}/{}", galaxyId, system.id), system);

        generateSectors(system.seed, galaxyId, system.id);
    }
}

void Generator::generateSectors(uint64_t seed, const std::string& galaxyId, const std::string& systemId) {
    const auto system = db.get<SystemData>(fmt::format("{}/{}", galaxyId, systemId)).value();

    Seeder seeder(seed);

    std::uniform_int_distribution<int> totalSectorsDist{options.systemSectorsMin, options.systemSectorsMax};
    const auto totalSectors = totalSectorsDist(seeder.rng());

    std::uniform_real_distribution<float> angleDist{0.0f, 360.0f};

    for (auto i = 0; i < totalSectors; i++) {
        auto pos = Vector2{static_cast<float>(i), 0.0f};
        const auto m = glm::rotate(Matrix4{1.0f}, glm::radians(angleDist(seeder.rng())), glm::vec3(0.0, 0.0, 1.0));
        pos = glm::vec3(m * glm::vec4(pos, 0.0f, 1.0));

        SectorData sector;
        sector.id = uuid();
        sector.systemId = systemId;
        sector.pos = pos;
        sector.name = fmt::format("{} {}", system.name, i + 1);
        sector.generated = false;

        db.put(fmt::format("{}/{}/{}", system.galaxyId, system.id, sector.id), sector);
    }
}
