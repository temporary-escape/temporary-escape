#include "Generator.hpp"

#include "../Utils/Random.hpp"
#include "World.hpp"

using namespace Scissio;

static const std::vector<std::string> REGION_SUFFIXES = {
    "Region", "District", "Province", "Place",  "Territory", "Zone",    "Division",
    "Domain", "Expanse",  "Realm",    "Sphere", "Vicinity",  "Enclave", "Area",
};

class GalaxyDistribution {
public:
    explicit GalaxyDistribution(const float maxWidth)
        : maxWidth(maxWidth), distArmIndex(0, ARMS_COUNT), distArmDistance(0.01f, 1.0f),
          distArmOffset(-ARM_ANGLE_HALF, ARM_ANGLE_HALF), distGridOffset(-0.4f, 0.4f) {
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

static constexpr auto GALAXY_WIDTH = 300.0f;

static std::vector<Region> generateRegions(const Config& config, World& world, const Galaxy& galaxy) {
    std::mt19937_64 rng{galaxy.seed};

    std::uniform_real_distribution<float> distPosition{-(GALAXY_WIDTH * 0.85f), GALAXY_WIDTH * 0.85f};

    const auto total = static_cast<int>(GALAXY_WIDTH / 50.0f) * 4;
    std::vector<Vector2> positions;

    // Not optimized but the number of regions will be very low.
    // Should be around 18
    const auto distanceCheck = [&](const Vector2& test) {
        for (const auto& pos : positions) {
            if (glm::distance(test, pos) < 50.0f) {
                return false;
            }
        }

        return true;
    };

    positions.push_back({0.0f, 0.0f});

    for (auto i = 1; i < total; i++) {
        while (true) {
            const Vector2 test = {
                distPosition(rng),
                distPosition(rng),
            };

            if (distanceCheck(test)) {
                positions.push_back(test);
                break;
            }
        }
    }

    const auto suffix = [&]() {
        std::uniform_int_distribution<size_t> dist(0, REGION_SUFFIXES.size() - 1);
        return REGION_SUFFIXES.at(dist(rng));
    };

    std::vector<Region> regions;

    const auto hueStep = 1.0f / positions.size();
    auto first = true;

    for (const auto& pos : positions) {
        const auto name = first ? "The Core" : randomName(rng) + " " + suffix();
        first = false;

        const auto hue = hueStep * regions.size();

        Region region{0, name, galaxy.id, pos.x, pos.y, hue};

        try {
            world.regions.insert(region);
        } catch (...) {
            EXCEPTION_NESTED("Failed to create new region '{}'", name);
        }

        regions.push_back(std::move(region));
    }

    return regions;
}

static std::vector<System> generateSystems(const Config& config, World& world, const Galaxy& galaxy,
                                           const std::vector<Region>& regions) {
    std::mt19937_64 rng{galaxy.seed};

    std::uniform_int_distribution<uint64_t> distSeed;

    GalaxyDistribution distGalaxy(GALAXY_WIDTH);

    std::vector<Vector2> positions;
    positions.resize(2000);

    std::vector<System> result;
    std::unordered_set<std::string> names;
    names.reserve(positions.size());
    result.reserve(positions.size());

    // Generate stars positions
    for (auto& pos : positions) {
        pos = distGalaxy(rng);
    }

    // Not optimized but the number of regions will be very low.
    // Should be around 18
    const auto nearestRegion = [&](const Vector2& pos) {
        assert(regions.size() > 0);

        const Region* ptr = nullptr;
        for (const auto& region : regions) {
            if (ptr == nullptr) {
                ptr = &region;
            } else {
                const auto a = glm::distance(pos, Vector2{region.posX, region.posY});
                const auto b = glm::distance(pos, Vector2{ptr->posX, ptr->posY});

                if (a < b) {
                    ptr = &region;
                }
            }
        }

        assert(ptr != nullptr);
        return *ptr;
    };

    std::unordered_map<uint64_t, std::vector<const System*>> regionsMap;

    // Generate stars data
    for (const auto& pos : positions) {
        // System seed
        const auto seed = distSeed(rng);

        // System name
        std::string name;
        while (true) {
            const auto test = randomName(rng);
            if (names.find(test) == names.end()) {
                name = test;
                names.insert(test);
                break;
            }
        }

        // Decide which region this system belongs to.
        const auto& region = nearestRegion(pos);

        System system{0, name, galaxy.id, region.id, pos.x, pos.y, seed};

        try {
            world.systems.insert(system);
            result.push_back(system);

            // This is OK because the "systems" vector is resized beforehand, won't reallocate.
            regionsMap[region.id].push_back(&result.back());
        } catch (...) {
            EXCEPTION_NESTED("Failed to create new system '{}'", name);
        }
    }

    std::unordered_set<uint64_t> linkPairs;

    const auto linkKeyOf = [](const System& a, const System& b) {
        return (a.id & 0xFFFFFFFFULL) | ((a.id & 0xFFFFFFFFULL) << 32);
    };

    const auto linkExists = [&](const System& a, const System& b) {
        const auto itA = linkPairs.find(linkKeyOf(a, b));
        const auto itB = linkPairs.find(linkKeyOf(b, a));

        return itA != linkPairs.end() && itB != linkPairs.end();
    };

    // Generate links for systems.
    // We will do this based on regions, so that
    // each region looks like a group of systems.
    // We will later connect these regions together.
    // Additionally, all systems in the region must form a connected graph.
    // I.e. the player must be able to travel to any system within the region.

    for (const auto& [regionId, systems] : regionsMap) {
        (void)regionId;

        // Traverse all systems and connect them together at exactly once
        // by choosing the nearest neighbor.

        for (size_t i = 1; i < systems.size(); i++) {
            auto system = systems.at(i - 1);
            const Vector2 systemPos{system->posX, system->posY};

            // Find nearest
            const System* nearest = nullptr;
            for (size_t j = i; j < systems.size(); j++) {
                auto test = systems.at(j);

                if (nearest == nullptr) {
                    nearest = test;
                    continue;
                }

                const auto a = glm::distance2(Vector2{nearest->posX, nearest->posY}, systemPos);
                const auto b = glm::distance2(Vector2{test->posX, test->posY}, systemPos);

                if (b < a) {
                    nearest = test;
                }
            }

            // Create connection
            linkPairs.insert(linkKeyOf(*system, *nearest));
            linkPairs.insert(linkKeyOf(*nearest, *system));

            SystemLink linkA{0, system->id, nearest->id};
            SystemLink linkB{0, nearest->id, system->id};

            try {
                world.systemLinks.insert(linkA);
                world.systemLinks.insert(linkB);
            } catch (...) {
                EXCEPTION_NESTED("Failed to generate link for systems {} <-> {}", system->id, nearest->id);
            }
        }
    }

    return result;
}

static std::vector<Sector> generateSectors(const Config& config, World& world, const Galaxy& galaxy,
                                           const System& system) {
    std::mt19937_64 rng{system.seed};

    std::uniform_int_distribution<uint64_t> distSeed;

    std::vector<Sector> sectors;

    for (auto i = 0; i < 5; i++) {
        // Sector seed
        const auto seed = distSeed(rng);

        // Sector name
        const auto name = fmt::format("{} - {}", system.name, i + 1);

        // Sector pos
        const auto pos = Vector2{i, 2.0f};

        Sector sector{0, name, system.id, pos.x, pos.y, seed};

        try {
            world.sectors.insert(sector);
        } catch (...) {
            EXCEPTION_NESTED("Failed to create new sector '{}'", name);
        }

        sectors.push_back(std::move(sector));
    }

    return sectors;
}

Galaxy Generator::generateGalaxy(const Config& config, World& world, const uint64_t seed) {
    try {
        auto galaxies = world.galaxies.get(1);
        if (galaxies.has_value()) {
            return std::move(galaxies.value());
        }

        Galaxy galaxy{0, "Galaxy", seed};

        world.transaction([&]() {
            world.galaxies.insert(galaxy);

            const auto regions = generateRegions(config, world, galaxy);
            const auto systems = generateSystems(config, world, galaxy, regions);

            for (const auto& system : systems) {
                (void)generateSectors(config, world, galaxy, system);
            }
        });

        return galaxy;
    } catch (...) {
        EXCEPTION_NESTED("Failed to generate galaxy with seed '{}'", seed);
    }
}
