#include "ServiceRegions.hpp"
#include "../../Utils/Random.hpp"

#define CMP "ServiceRegions"

using namespace Engine;

static const std::vector<std::string> regionSuffixes = {
    "Region", "District", "Province", "Place",  "Territory", "Zone",    "Division",
    "Domain", "Expanse",  "Realm",    "Sphere", "Vicinity",  "Enclave", "Area",
};

ServiceRegions::ServiceRegions(const Config& config, AssetManager& assetManager, Engine::Database& db)
    : config(config), assetManager(assetManager), db(db) {
}

void ServiceRegions::tick() {
}

std::vector<RegionData> ServiceRegions::getForPlayer(const std::string& playerId, const std::string& galaxyId,
                                                     const std::string& start, std::string& next) {
    return db.next<RegionData>(fmt::format("{}/", galaxyId), start, 64, &next);
}

void ServiceRegions::generate() {
    const auto galaxies = db.seek<GalaxyData>("");
    for (const auto& galaxy : galaxies) {
        generate(galaxy.id);
    }
}

void ServiceRegions::generate(const std::string& galaxyId) {
    Log::i(CMP, "Generating regions for galaxy: '{}' ...", galaxyId);

    const auto& options = config.generator;

    const auto galaxyOpt = db.get<GalaxyData>(galaxyId);
    if (!galaxyOpt) {
        EXCEPTION("No such galaxy: '{}'", galaxyId);
    }

    const auto& galaxy = galaxyOpt.value();

    const auto test = db.seek<RegionData>(fmt::format("{}/", galaxy.id), 1);
    if (!test.empty()) {
        Log::i(CMP, "Already generated regions for galaxy: '{}' ...", galaxyId);
        return;
    }

    std::mt19937_64 rng(galaxy.seed);

    const auto maxRange = options.galaxyWidth - options.regionDistance / 2.0f;

    std::vector<Vector2> positions;
    auto tries = 20;

    while (tries > 0) {
        tries--;

        const auto randomPos =
            Vector2{randomReal<float>(rng, -maxRange, maxRange), randomReal<float>(rng, -maxRange, maxRange)};

        auto accepted = true;

        for (const auto& pos : positions) {
            if (glm::distance(randomPos, pos) < options.regionDistance) {
                accepted = false;
                break;
            }
        }

        if (!accepted) {
            continue;
        }

        positions.push_back(randomPos);
        tries = 5;
    }

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

        db.put(fmt::format("{}/{}", galaxy.id, region.id), region);

        regions.push_back(std::move(region));
    }

    Log::i(CMP, "Generated {} regions for galaxy '{}'", regions.size(), galaxy.name);
}
