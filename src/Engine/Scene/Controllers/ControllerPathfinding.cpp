#include "ControllerPathfinding.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../Scene.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerPathfinding::ControllerPathfinding(Scene& scene, entt::registry& reg, DynamicsWorld& dynamicsWorld) :
    scene{scene}, reg{reg}, dynamicsWorld{dynamicsWorld}, octree{*this, 6, 32} {
}

ControllerPathfinding::~ControllerPathfinding() = default;

void ControllerPathfinding::update(const float delta) {
    (void)delta;
}

void ControllerPathfinding::recalculate(VulkanRenderer& vulkan) {
    (void)vulkan;
}

void ControllerPathfinding::buildTree() {
    octree.build();
}

bool ControllerPathfinding::contactTestBox(const Vector3i& pos, const int width) {
    return dynamicsWorld.contactTestBox(pos, static_cast<float>(width), CollisionGroup::Static);
}

void ControllerPathfinding::debug(Scene& scene) {
    /*octree.iterate([this, &scene](const Vector3i& pos, int width) {
        auto entity = scene.createEntity();
        auto& transform = entity.addComponent<ComponentTransform>();
        transform.scale(Vector3{static_cast<float>(width / 2)});
        transform.move(pos);
        transform.setStatic(true);

        const auto model = AssetsManager::instance->getModels().find("model_cube");
        entity.addComponent<ComponentModel>(model);
    });*/
}
