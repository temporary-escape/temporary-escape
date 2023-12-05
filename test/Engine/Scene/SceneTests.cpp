#include "../../Common.hpp"
#include <Engine/Scene/Controllers/ControllerDynamicsWorld.hpp>
#include <Engine/Scene/Controllers/ControllerPathfinding.hpp>
#include <Engine/Scene/Scene.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

class SceneFixture {
public:
    SceneFixture() {
        config.assetsPath = Path{ROOT_DIR} / "assets";
        scene = std::make_unique<Scene>(config);
    }

    Config config;
    std::unique_ptr<Scene> scene;
};

TEST_CASE_METHOD(SceneFixture, "Check if box overlaps with dynamics world", "[Scene]") {
    auto sphere = CollisionShape::createSphere(1.0f);

    auto entity = scene->createEntity();
    auto& transform = entity.addComponent<ComponentTransform>();
    transform.move({5.0f, 0.0f, 0.0f});
    transform.setStatic(true);

    auto& rigidBody = entity.addComponent<ComponentRigidBody>();
    rigidBody.setMass(0.0f);

    auto& dynamicsWorld = scene->getController<ControllerDynamicsWorld>();
    dynamicsWorld.updateAabbs();

    SECTION("With radius 1.0f") {
        rigidBody.setScale(1.0f);
        rigidBody.setShape(sphere);

        REQUIRE(dynamicsWorld.contactTestSphere({5.0f, 1.0f, 0.0f}, 1.0f, CollisionGroup::Static) == true);
        REQUIRE(dynamicsWorld.contactTestSphere({5.0f, 1.0f, 0.0f}, 1.0f, CollisionGroup::Default) == false);
        REQUIRE(dynamicsWorld.contactTestSphere({5.0f, 3.0f, 0.0f}, 1.0f, CollisionGroup::Static) == false);
    }

    SECTION("With radius 2.5f") {
        rigidBody.setScale(2.5f);
        rigidBody.setShape(sphere);

        REQUIRE(dynamicsWorld.contactTestSphere({5.0f, 1.0f, 0.0f}, 1.0f, CollisionGroup::Static) == true);
        REQUIRE(dynamicsWorld.contactTestSphere({5.0f, 1.0f, 0.0f}, 1.0f, CollisionGroup::Default) == false);
        REQUIRE(dynamicsWorld.contactTestSphere({5.0f, 3.0f, 0.0f}, 1.0f, CollisionGroup::Static) == true);
        REQUIRE(dynamicsWorld.contactTestSphere({5.0f, 5.0f, 0.0f}, 1.0f, CollisionGroup::Static) == false);
    }
}

TEST_CASE_METHOD(SceneFixture, "Build pathfinding tree and find node", "[Scene]") {
    auto sphere = CollisionShape::createSphere(1.0f);

    for (auto z = -128; z <= 128; z += 32) {
        auto entity = scene->createEntity();
        auto& transform = entity.addComponent<ComponentTransform>();
        transform.move({5.0f, 5.0f, static_cast<float>(z) + 16.0f});
        // transform.setStatic(true);

        auto& rigidBody = entity.addComponent<ComponentRigidBody>();
        // rigidBody.setMass(0.0f);
        rigidBody.setScale(1.0f);
        rigidBody.setShape(sphere);
    }

    auto& dynamicsWorld = scene->getController<ControllerDynamicsWorld>();
    dynamicsWorld.updateAabbs();

    auto& pathfinding = scene->getController<ControllerPathfinding>();
    pathfinding.buildTree();

    REQUIRE(pathfinding.getOctree().find({5.0f, 5.0f, -128.0f + 16.0f}) != 0);
    REQUIRE(pathfinding.getOctree().find({5.0f, 64.0f, -128.0f + 16.0f}) == 0);
}
