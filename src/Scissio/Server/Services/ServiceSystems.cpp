#include "ServiceSystems.hpp"
#include "../../Utils/Random.hpp"

#define CMP "ServiceSystem"

using namespace Scissio;

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

ServiceSystems::ServiceSystems(const Config& config, AssetManager& assetManager, Scissio::Database& db)
    : config(config), assetManager(assetManager), db(db) {
}

void ServiceSystems::tick() {
}

std::vector<SystemData> ServiceSystems::getForPlayer(const std::string& playerId, const std::string& galaxyId,
                                                     const std::string& start, std::string& next) {
    return db.next<SystemData>(fmt::format("{}/", galaxyId), start, 64, &next);
}

void ServiceSystems::generate() {
    const auto galaxies = db.seek<GalaxyData>("");
    for (const auto& galaxy : galaxies) {
        generate(galaxy.id);
    }
}

void ServiceSystems::generate(const std::string& galaxyId) {
    const auto& options = config.generator;

    Log::i(CMP, "Generating systems for galaxy: '{}' ...", galaxyId);

    const auto galaxyOpt = db.get<GalaxyData>(galaxyId);
    if (!galaxyOpt) {
        EXCEPTION("No such galaxy: '{}'", galaxyId);
    }

    const auto& galaxy = galaxyOpt.value();

    const auto test = db.seek<SystemData>(fmt::format("{}/", galaxy.id), 1);
    if (!test.empty()) {
        Log::i(CMP, "Already generated systems for galaxy: '{}' ...", galaxyId);
        return;
    }

    const auto regions = db.seek<RegionData>(fmt::format("{}/", galaxy.id));
    if (regions.empty()) {
        EXCEPTION("Number of regions in galaxy '{}' must not be zero", galaxy.id);
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

    std::mt19937_64 rng(galaxy.seed);

    GalaxyDistribution distributor(options.galaxyWidth);

    std::unordered_set<std::string> namesTaken;
    namesTaken.reserve(options.totalSystems);

    std::vector<SystemData> systems;
    systems.reserve(options.totalSystems);

    for (auto i = 0; i < options.totalSystems; i++) {
        const auto pos = distributor(rng);
        const auto region = getNearestRegion(pos);

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
        system.regionId = region.id;
        system.name = name;
        system.pos = pos;
        system.seed = randomInt<uint64_t>(rng);

        db.put(fmt::format("{}/{}", galaxy.id, system.id), system);

        systems.push_back(system);
    }

    Log::i(CMP, "Generated {} systems for galaxy: '{}'", systems.size(), galaxy.name);
}
