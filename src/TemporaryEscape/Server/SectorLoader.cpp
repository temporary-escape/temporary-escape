#include "SectorLoader.hpp"
#include "../Utils/Random.hpp"

using namespace Engine;

class SpaceTester {
public:
    bool addSphere(const Vector3& pos, float radius) {
        for (const auto& [otherPos, otherRadius] : objects) {
            const auto dist = glm::distance(pos, otherPos);
            if (dist < otherRadius + radius) {
                return false;
            }
        }

        objects.emplace_back(pos, radius);
        return true;
    }

private:
    std::vector<std::tuple<Vector3, float>> objects;
};

struct AsteroidCluster {
    Vector3 origin;
    float radius{1.0f};
    float sizeMin{0.5f};
    float sizeMax{5.0f};
    int count{100};
    std::vector<std::tuple<float, AssetAsteroidPtr>> weights;

    void generate(Scene& scene, const uint64_t seed) {
        SpaceTester tester;
        std::mt19937_64 rng{seed};

        const auto randomPointInSphere = [&](const float size) -> std::optional<Vector3> {
            // https://stackoverflow.com/a/5408843
            auto tries = 10;
            while (tries-- > 0) {
                const auto phi = randomReal(rng, 0.0f, 2.0f * glm::pi<float>());
                const auto costheta = randomReal(rng, -1.0f, 1.0f);
                const auto u = randomReal(rng, 0.0f, 1.0f);

                const auto theta = glm::acos(costheta);
                // TODO: Needs fix, currently points are clustering at the middle.
                // r = R * cuberoot( u )
                const auto r = (radius - size) * u;
                Vector3 pos;

                pos.x = r * glm::sin(theta) * glm::cos(phi);
                pos.y = r * glm::sin(theta) * glm::sin(phi);
                pos.z = r * glm::cos(theta);

                if (tester.addSphere(pos, size)) {
                    return pos;
                }
            }

            return std::nullopt;
        };

        const auto chooseAsteroid = [&]() -> auto& {
            // TODO: Choose by weights.
            if (weights.empty()) {
                EXCEPTION("No weights");
            }
            return std::get<1>(weights.front());
        };

        const auto chooseModel = [&](const AssetAsteroidPtr& asteroid) -> auto& {
            // TODO: Choose models by weights.
            if (asteroid->getModels().empty()) {
                EXCEPTION("Asset '{}' has no models", asteroid->getName());
            }
            const auto idx = randomInt<size_t>(rng, 0UL, asteroid->getModels().size() - 1);
            return asteroid->getModels().at(idx).model;
        };

        for (auto i = 0; i < count; i++) {
            const auto size = randomReal(rng, sizeMin, sizeMax);
            const auto pos = randomPointInSphere(size);

            if (!pos.has_value()) {
                break;
            }

            const auto& asteroid = chooseAsteroid();

            auto entity = std::make_shared<Entity>();
            entity->translate(pos.value() + origin);
            entity->scale(Vector3{size});
            entity->rotate(randomQuaternion(rng));

            auto model = entity->addComponent<ComponentModel>(chooseModel(asteroid));

            scene.addEntity(entity);
        }
    }
};

SectorLoader::SectorLoader(const Config& config, AssetManager& assetManager, Database& db, Services& services)
    : config(config), assetManager(assetManager), db(db), services(services) {
}

void SectorLoader::populate(const std::string& galaxyId, const std::string& systemId, const std::string& sectorId,
                            Scene& scene) {
    const auto compoundId = fmt::format("{}/{}/{}", galaxyId, systemId, sectorId);
    auto found = db.get<SectorData>(compoundId);
    if (!found) {
        EXCEPTION("Unable to populate sector: '{}' not found", compoundId);
    }

    const auto& sector = found.value();

    std::mt19937_64 rng{sector.seed};

    auto skybox = std::make_shared<Entity>();
    skybox->addComponent<ComponentSkybox>(sector.seed);
    skybox->scale(Vector3{1000.0f});
    scene.addEntity(skybox);

    auto sun = std::make_shared<Entity>();
    sun->addComponent<ComponentDirectionalLight>(Color4{1.0f, 0.9f, 0.8f, 1.0f} * 3.0f);
    sun->translate(Vector3{-2.0f, 2.0f, 2.0f});
    scene.addEntity(sun);

    auto blockEngineHousing = assetManager.find<AssetBlock>("block_engine_housing_t1");
    auto blockEngineNozzle = assetManager.find<AssetBlock>("block_engine_nozzle_t1");
    auto blockFrame = assetManager.find<AssetBlock>("block_frame_t1");
    auto blockBattery = assetManager.find<AssetBlock>("block_battery_t1");
    auto blockSmallReactor = assetManager.find<AssetBlock>("block_reactor_small_t1");
    auto blockTank = assetManager.find<AssetBlock>("block_tank_t1");

    auto dummy = std::make_shared<Entity>();
    auto grid = dummy->addComponent<ComponentGrid>();
    grid->insert(blockEngineHousing, Vector3i{-1, 0, 0}, 0);
    grid->insert(blockEngineHousing, Vector3i{1, 0, 0}, 0);
    grid->insert(blockEngineNozzle, Vector3i{-1, 0, 1}, 0);
    grid->insert(blockEngineNozzle, Vector3i{1, 0, 1}, 0);
    grid->insert(blockFrame, Vector3i{0, 0, 0}, 0);
    grid->insert(blockBattery, Vector3i{-1, 0, -1}, 0);
    grid->insert(blockBattery, Vector3i{1, 0, -1}, 0);
    grid->insert(blockSmallReactor, Vector3i{0, 0, -1}, 0);
    grid->insert(blockTank, Vector3i{-1, 0, -2}, 0);
    grid->insert(blockTank, Vector3i{1, 0, -2}, 0);
    grid->insert(blockFrame, Vector3i{0, 0, -2}, 0);

    auto turretAsset = assetManager.find<AssetTurret>("turret_01");
    auto turret = dummy->addComponent<ComponentTurret>(turretAsset);
    // turret->setOffset(Vector3{0, 1, -2});
    turret->setTarget(Vector3{-4, 6, -15});

    dummy->rotate(Vector3{0.0f, 1.0f, 0.0f}, 45.0f);
    dummy->rotate(Vector3{1.0f, 0.0f, 0.0f}, -25.0f);
    dummy->translate(Vector3{3.0f, 0.0f, 0.0f});

    auto control = dummy->addComponent<ComponentShipControl>();
    control->init();

    scene.addEntity(dummy);

    dummy = std::make_shared<Entity>();
    dummy->addComponent<ComponentModel>(blockFrame->getModel());
    dummy->translate(turret->getTarget());
    scene.addEntity(dummy);

    auto asteroidRock = assetManager.find<AssetAsteroid>("asteroid_rock");

    AsteroidCluster asteroidCluster{};
    asteroidCluster.origin = Vector3{-200.0f, 0.0f, 0.0f};
    asteroidCluster.radius = 200.0f;
    asteroidCluster.count = 500;
    asteroidCluster.weights.emplace_back(1.0f, asteroidRock);
    asteroidCluster.generate(scene, randomInt<uint64_t>(rng));
}
