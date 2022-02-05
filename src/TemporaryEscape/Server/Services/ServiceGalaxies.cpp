#include "ServiceGalaxies.hpp"
#include "../../Utils/Random.hpp"

#define CMP "ServiceGalaxies"

using namespace Engine;

ServiceGalaxies::ServiceGalaxies(const Config& config, AssetManager& assetManager, Engine::Database& db)
    : config(config), assetManager(assetManager), db(db) {
}

void ServiceGalaxies::tick() {
}

void ServiceGalaxies::generate(const uint64_t seed) {
    Log::i(CMP, "Generating galaxies ...");

    const auto test = db.seek<GalaxyData>("");
    if (!test.empty()) {
        Log::i(CMP, "Already generated galaxies");
        return;
    }

    std::mt19937_64 rng(seed);

    GalaxyData galaxy;
    galaxy.id = uuid();
    galaxy.name = fmt::format("{} Galaxy", randomName(rng));
    galaxy.pos = {0.0f, 0.0f};
    galaxy.seed = randomInt<uint64_t>(rng);

    createGalaxy(galaxy);
}

GalaxyData ServiceGalaxies::getForPlayer(const std::string& playerId, const std::string& id) {
    auto galaxy = db.get<GalaxyData>(id);
    if (!galaxy) {
        EXCEPTION("Galaxy {} does not exist", id);
    }
    return galaxy.value();
}

void ServiceGalaxies::createGalaxy(const GalaxyData& galaxy) {
    db.put(galaxy.id, galaxy);
}
