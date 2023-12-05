#include "ControllerPathfinding.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../Scene.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerPathfinding::ControllerPathfinding(entt::registry& reg, ControllerDynamicsWorld& dynamicsWorld) :
    reg{reg}, dynamicsWorld{dynamicsWorld}, octree{*this, 8, 64} {
}

ControllerPathfinding::~ControllerPathfinding() = default;

void ControllerPathfinding::update(const float delta) {
    (void)delta;
}

void ControllerPathfinding::recalculate(VulkanRenderer& vulkan) {
    (void)vulkan;
}

void ControllerPathfinding::buildTree() {
    const auto t0 = std::chrono::high_resolution_clock::now();
    octree.build();
    const auto t1 = std::chrono::high_resolution_clock::now();
    const auto diff = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);
    logger.info("Built pathfinding graph in {}us with {} nodes", diff.count(), octree.getCount());
}

bool ControllerPathfinding::contactTestBox(const Vector3& pos, float width) {
    return dynamicsWorld.contactTestBox(pos, width, CollisionGroup::Static);
}

void ControllerPathfinding::debug(Scene& scene) {
    octree.iterate([this, &scene](const Vector3i& pos, int width) {
        auto entity = scene.createEntity();
        auto& transform = entity.addComponent<ComponentTransform>();
        transform.scale(Vector3{static_cast<float>(width / 2)});
        transform.move(pos);
        transform.setStatic(true);

        const auto model = AssetsManager::instance->getModels().find("model_cube");
        entity.addComponent<ComponentModel>(model);
    });
}
